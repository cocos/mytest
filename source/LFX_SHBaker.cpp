#include "LFX_SHBaker.h"
#include "LFX_World.h"

namespace LFX {

	void SHCalcLighting(Float3& result, const Vertex& V, Light* L, const Material* M)
	{
		float kl = 0;
		Float3 color;
		World::Instance()->GetShader()->DoLighting(color, kl, V, L, M);
		if (kl >= 0) {
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

	void SHBaker::Run(SHProbe* probe)
	{
		std::vector<Float3> results;
		std::vector<Float3> samples = LightProbeSampler::uniformSampleSphereAll(32, 32);
		for (int sampleIdx = 0; sampleIdx < samples.size(); ++sampleIdx) {
			Ray ray;
			ray.orig = probe->position;
			ray.dir = samples[sampleIdx];
			
			Contact contact;
			if (World::Instance()->GetScene()->RayCheck(contact, ray, DEFAULT_RAYTRACE_MAX_LENGHT, LFX_TERRAIN | LFX_MESH)) {
				// back facing
				if (!contact.facing) {
					results.push_back(Float3(0, 0, 0));
					continue;
				}

				Vertex vtx = contact.vhit;
				Material* mtl = (Material*)contact.mtl;
				const float lenSq = (vtx.Position - ray.orig).lenSqr();

				Float3 result;
				std::vector<Light*> lights;
				SHGetLightList(lights, vtx.Position);
				for (auto* light : lights) {
					SHCalcLighting(result, vtx, light, mtl);
				}

				results.push_back(result);
			}
			else {
				results.push_back(_cfg.SkyRadiance);
			}
		}

		auto radianceCoefficients = SH::project(LightProbeQuality::Normal, samples, results);
		auto irradianceCoefficients = SH::convolveCosine(LightProbeQuality::Normal, radianceCoefficients);
		probe->coefficients = irradianceCoefficients;
	}

}