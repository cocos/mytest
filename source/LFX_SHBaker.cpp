#include "LFX_SHBaker.h"
#include "LFX_ILBakerSampling.h"
#include "LFX_ILPathTrace.h"
#include "LFX_World.h"

namespace LFX {

	void SHCalcDirectLighting(Float3& result, const Vertex& V, Light* L, const Material* M)
	{
		float kl = 0;
		Float3 color;
		World::Instance()->GetShader()->DoLighting(color, kl, V, L, M, false, false);
		if (kl >= 0 && L->CastShadow) {
			float len = 0;
			Ray ray;

			if (L->Type != Light::DIRECTION) {
				ray.dir = L->Position - V.Position;
				len = ray.dir.len();
				ray.dir.normalize();
			}
			else {
				ray.dir = -L->Direction;
				len = FLT_MAX;
			}

			ray.orig = V.Position + ray.dir * UNIT_LEN * 0.01f;

			if (len > 0.01f * UNIT_LEN) {
				Contact contract;
				if (World::Instance()->GetScene()->Occluded(ray, len, LFX_TERRAIN | LFX_MESH)) {
					kl = 0;
				}
			}
		}

		result += kl > 0 ? color * L->DirectScale : Float3(0, 0, 0);
	}

	float SHCalcIndirectLighting(Float3& result, const Vertex& V, Light* L, const Material* M)
	{
		float kl = 0;
		Float3 color;
		World::Instance()->GetShader()->DoLighting(color, kl, V, L, M, true, true);
		if (kl > 0 && L->CastShadow) {
			float len = 0;
			Ray ray;

			if (L->Type != Light::DIRECTION) {
				ray.dir = L->Position - V.Position;
				len = ray.dir.len();
				ray.dir.normalize();
			}
			else {
				ray.dir = -L->Direction;
				len = FLT_MAX;
			}

			ray.orig = V.Position + ray.dir * UNIT_LEN * 0.01f;

			if (len > 0.01f * UNIT_LEN) {
				Contact contract;
				if (World::Instance()->GetScene()->Occluded(ray, len, LFX_TERRAIN | LFX_MESH)) {
					kl = 0;
				}
			}
		}

		result += kl > 0 ? color * L->IndirectScale : Float3(0, 0, 0);
		return kl;
	}

	void SHGetLightList(std::vector<Light*>& lights, const Float3& point)
	{
		for (auto* light : World::Instance()->Lights()) {
			if (!light->GIEnable) {
				continue;
			}

			if (IsLightVisible(light, point)) {
				lights.push_back(light);
			}
		}
	}

	using namespace ILBaker;

	bool SHPathTraceFunc(Float3& result, const PathTraceParams& params, const Vertex& vtx, const Material* mtl, bool hitSky)
	{
		if (hitSky) {
			result += params.skyRadiance;
			return true;
		}

		float kl = 0;
		std::vector<Light*> lights;
		SHGetLightList(lights, vtx.Position);
		for (size_t i = 0; i < lights.size(); ++i) {
			Float3 diffuse;
			kl += SHCalcIndirectLighting(diffuse, vtx, lights[i], mtl);
			result += diffuse * params.diffuseScale;
		}

		return kl > 0;
	}

	PathTraceResult SHPathTrace(const PathTraceParams& params, Random& rand, bool& hitSky)
	{
		PathTraceResult result;

		Ray ray;
		ray.orig = params.rayStart;
		ray.dir = params.rayDir;

		// Keep tracing paths until we reach the specified max
		float throughput = 1.0f;
		Entity* traceEntity = params.entity;
		const int maxPathLength = params.maxPathLength;
		for (; result.pathLen <= maxPathLength || maxPathLength == -1; ++result.pathLen) {
#if 0 // Disable russian roulette
			// See if we should randomly terminate this path using Russian Roullete
			const int rouletteDepth = params.RussianRouletteDepth;
			if (pathLength >= rouletteDepth && rouletteDepth != -1) {
				float continueProbability = std::min<float>(params.RussianRouletteProbability, ComputeLuminance(throughput));
				if (randomGenerator.RandomFloat() > continueProbability) {
					break;
				}
				throughput /= continueProbability;
				irrThroughput /= continueProbability;
			}
#endif

			// Set this to true to keep the loop going
			bool continueTracing = false;

			// Check for intersection with the scene
			Contact contact;
			if (World::Instance()->GetScene()->RayCheck(contact, ray, params.rayLen, LFX_TERRAIN | LFX_MESH)) {
				traceEntity = contact.entity;

				// back facing
				if (!contact.facing) {
					break;
				}

				Vertex vtx = contact.vhit;
				Material* mtl = (Material*)contact.mtl;

				Float3 diffuse;
				if (!SHPathTraceFunc(diffuse, params, vtx, mtl, false)) {
					break;
				}

				float lenSq = (vtx.Position - ray.orig).lenSqr();
				//throughput *= 1.0f / (lenSq + 1.0f);
				throughput *= 1.0f / Pi;
				result.color += (diffuse * throughput);

				// Pick a new path, using MIS to sample both our diffuse and specular BRDF's
				if (1) {
					Mat3 tangentToWorld;
					tangentToWorld.SetXBasis(vtx.Tangent);
					tangentToWorld.SetYBasis(vtx.Binormal);
					tangentToWorld.SetZBasis(vtx.Normal);

					Float2 sample;
#if 0
					// Randomly select if we should sample our diffuse BRDF, or our specular BRDF
					if (params.sampleSet && result.pathLen <= 1) {
						sample = params.sampleSet->BRDF();
					}
					else {
						sample = rand.RandomFloat2();
					}
#else
					sample = rand.RandomFloat2();
#endif

					//Float3 v = Float3::Normalize(rayOrigin - hitSurface.position);

					// We're sampling the diffuse BRDF, so sample a cosine-weighted hemisphere
					Float3 sampleDir;
					sampleDir = SampleCosineHemisphere(sample);
					sampleDir = Float3::Normalize(Mat3::Transform(sampleDir, tangentToWorld));
					if (sampleDir.dot(vtx.Normal) > 0.0f) {
						// Generate the ray for the new path
						ray.orig = vtx.Position + sampleDir * 0.01f;
						ray.dir = sampleDir;

						continueTracing = true;
					}
				}
			}
			else {
				Float3 diffuse;
				if (SHPathTraceFunc(diffuse, params, Vertex(), nullptr, true)) {
					result.color += diffuse * throughput;
				}
				hitSky = true;
			}

			if (continueTracing == false) break;
		}

		return result;
	}

	void SHBaker::Run(SHProbe* probe)
	{
		_ctx.Samples = World::Instance()->GetSetting()->GIProbeSamples;
		_ctx.LightingScale = World::Instance()->GetSetting()->GIProbeScale;
		_ctx.MaxPathLength = World::Instance()->GetSetting()->GIProbePathLength;
		_ctx.SkyRadiance = World::Instance()->GetSetting()->SkyRadiance;

		std::vector<Float3> radianceCoefficients;

		// Calculate indirect lightings
		{
			std::vector<Float3> results;
			std::vector<Float3> samples = LightProbeSampler::uniformSampleSphereAll(_ctx.Samples);

			for (int sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
				Ray ray;
				ray.orig = probe->position;
				ray.dir = samples[sampleIdx];

				PathTraceParams params;
				params.sampleSet = nullptr;
				params.rayDir = ray.dir;
				params.rayStart = ray.orig;
				params.rayLen = DEFAULT_RAYTRACE_MAX_LENGHT;
				params.maxPathLength = _ctx.MaxPathLength;
				//params.russianRouletteDepth = _ctx.RussianRouletteDepth;
				//params.russianRouletteProbability = 0.5f;
				params.skyRadiance = _ctx.SkyRadiance;
				params.diffuseScale = _ctx.LightingScale;

				bool hitSky = false;
				PathTraceResult sampleResult = SHPathTrace(params, _ctx.Random, hitSky);

				std::vector<Light*> lights;
				SHGetLightList(lights, probe->position);
				for (auto* light : lights) {
					if (light->Type == Light::DIRECTION) {
						continue;
					}
					if (light->DirectScale <= 0) {
						continue;
					}

					Vertex vtx;
					Material mat;

					vtx.Position = probe->position;
					vtx.Normal = ray.dir;

					Float3 color;
					SHCalcDirectLighting(color, vtx, light, &mat);
					sampleResult.color += color;
				}

				results.push_back(sampleResult.color);
			}

			radianceCoefficients = SH::project(samples, results);
		}

		// Calculate direct lightings
#if 0
		{
            std::vector<Float3> results;
			std::vector<Float3> samples;

			std::vector<Light*> lights;
			SHGetLightList(lights, probe->position);
			for (auto* light : lights) {
				if (light->Type == Light::DIRECTION) {
					continue;
				}
				if (light->DirectScale <= 0) {
					continue;
				}

				Vertex vtx;
				Material mat;

				vtx.Position = probe->position;
				vtx.Normal = Float3::Normalize(light->Position - probe->position);

				Float3 result;
				SHCalcDirectLighting(result, vtx, light, &mat);
				results.push_back(result);
				samples.push_back(vtx.Normal);
			}

			if (!samples.empty()) {
				auto directRadianceCoefficients = SH::project(samples, results);
				assert(radianceCoefficients.size() == directRadianceCoefficients.size());

				for (auto i = 0; i < directRadianceCoefficients.size(); i++) {
					radianceCoefficients[i] += directRadianceCoefficients[i];
				}
			}
		}
#endif

		auto irradianceCoefficients = SH::convolveCosine(radianceCoefficients);
		probe->coefficients = irradianceCoefficients;
	}

}
