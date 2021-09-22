#include "LFX_World.h"
#include "LFX_ILBakerRaytrace.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	inline float ComputeLuminance(Float3 color)
	{
		return Float3::Dot(color, Float3(0.2126f, 0.7152f, 0.0722f));
	}

	static float CosineWeightedMonteCarloFactor(int numSamples)
	{
		// Integrating cosine factor about the hemisphere gives you Pi, and the PDF
		// of a cosine-weighted hemisphere function is 1 / Pi.
		// So the final monte-carlo weighting factor is 1 / NumSamples
		return (1.0f / numSamples);
	}

	static float HemisphereMonteCarloFactor(int numSamples)
	{
		// The area of a unit hemisphere is 2 * Pi, so the PDF is 1 / (2 * Pi)
		return ((2.0f * Pi) / numSamples);
	}

	static float SphereMonteCarloFactor(int numSamples)
	{
		// The area of a unit hemisphere is 2 * Pi, so the PDF is 1 / (2 * Pi)
		return ((4.0f * Pi) / numSamples);
	}

	struct DiffuseBaker
	{
		static const int BasisCount = 1;

		int NumSamples = 0;
		Float3 ResultSum;

		void Init(int numSamples)
		{
			NumSamples = numSamples;
			ResultSum = Float3(0, 0, 0);
		}

		Float3 SampleDirection(Float2 samplePoint)
		{
			return ILBaker::SampleCosineHemisphere(samplePoint.x, samplePoint.y);
		}

		void AddSample(Float3 sampleDir, int sampleIdx, Float3 sample, bool hitSky)
		{
			ResultSum += sample;
		}

		void FinalResult(Float4 bakeOutput[BasisCount])
		{
			Float3 finalResult = ResultSum * CosineWeightedMonteCarloFactor(NumSamples);
			bakeOutput[0].x = Clamp<float>(finalResult.x, 0.0f, FP16Max);
			bakeOutput[0].y = Clamp<float>(finalResult.y, 0.0f, FP16Max);
			bakeOutput[0].z = Clamp<float>(finalResult.z, 0.0f, FP16Max);
			bakeOutput[0].w = 1;
		}

		void ProgressiveResult(Float4 bakeOutput[BasisCount], int passIdx)
		{
			const float lerpFactor = passIdx / (passIdx + 1.0f);
			Float3 newSample = ResultSum * CosineWeightedMonteCarloFactor(1);
			Float3 currValue = Float3(bakeOutput[0].x, bakeOutput[0].y, bakeOutput[0].z);
			currValue = Lerp<Float3>(newSample, currValue, lerpFactor);
			bakeOutput[0].x = Clamp<float>(currValue.x, 0.0f, FP16Max);
			bakeOutput[0].y = Clamp<float>(currValue.y, 0.0f, FP16Max);
			bakeOutput[0].z = Clamp<float>(currValue.z, 0.0f, FP16Max);
			bakeOutput[0].w = 1;
		}
	};

	enum class IntegrationTypes
	{
		Pixel = 0,
		Lens,
		BRDF,
		Sun,
		AreaLight,

		NumValues,
	};

	struct IntegrationSampleSet
	{
		Float2 Samples[ILBakerRaytrace::NumIntegrationTypes];

		void Init(const ILBaker::IntegrationSamples & samples, int pixelIdx, int sampleIdx)
		{
			assert(samples.NumTypes == ILBakerRaytrace::NumIntegrationTypes);
			samples.GetSampleSet(pixelIdx, sampleIdx, Samples);
		}

		Float2 Pixel() const { return Samples[int(IntegrationTypes::Pixel)]; }
		Float2 Lens() const { return Samples[int(IntegrationTypes::Lens)]; }
		Float2 BRDF() const { return Samples[int(IntegrationTypes::BRDF)]; }
		Float2 Sun() const { return Samples[int(IntegrationTypes::Sun)]; }
		Float2 AreaLight() const { return Samples[int(IntegrationTypes::AreaLight)]; }
	};

	struct PathTracerParams
	{
		Float3 RayStart;
		Float3 RayDir;
		float RayLen = 0.0f;
		int MaxPathLength = -1;
#if 0
		int RussianRouletteDepth = 4;
		float RussianRouletteProbability = 0.5f;
#endif
		const IntegrationSampleSet* SampleSet = nullptr;

		float DiffuseScale;
		Float3 SkyRadiance;
	};

	//
	void RT_CalcuLighting(const Float3 & P, const Vertex & V, Light * L, Float3 & diffuse, Float3 & irradiance)
	{
		Float3 PS = V.Position - P;
		float lenSq = PS.lenSqr();

		float kd = 0, ka = 0, ks = 0;
		DoLighting(kd, ka, ks, V, L);

		if (kd * ka * ks >= 0)
		{
			float len = 0;
			Ray ray;

			if (L->Type != Light::DIRECTION)
			{
				ray.dir = L->Position - V.Position;
				len = ray.dir.len();
				ray.dir.normalize();
			}
			else
			{
				ray.dir = -L->Direction;
				len = FLT_MAX;
			}

			ray.orig = V.Position + ray.dir * UNIT_LEN * 0.01f;

			if (len > 0.01f * UNIT_LEN)
			{
				Contact contract;
				if (World::Instance()->GetScene()->Occluded(ray, len, LFX_TERRAIN | LFX_MESH))
				{
					kd = ka = ks = 0;
				}
			}
		}

		float invRSqr = 1.0f / (lenSq * lenSq);

		diffuse += kd * ka * ks * L->Color * L->IndirectScale;
		irradiance += diffuse * invRSqr;
	}

	void GetLightList(std::vector<Light *> & lights, const Float3 & point)
	{
		for (int j = 0; j < World::Instance()->GetLightCount(); ++j)
		{
			Light * light = World::Instance()->GetLight(j);
			if (!light->GIEnable)
				continue;

			if (IsLightVisible(light, point))
			{
				lights.push_back(light);
			}
		}
	}

	Float3 PathTrace(const PathTracerParams& params, ILBaker::Random& randomGenerator, float& illuminance, bool& hitSky)
	{
		Ray ray;
		ray.orig = params.RayStart;
		ray.dir = params.RayDir;

		Float3 radiance = Float3(0, 0, 0);
		Float3 irradiance = Float3(0, 0, 0);
		Float3 throughput = Float3(1.0f, 1.0f, 1.0f);
		Float3 irrThroughput = Float3(1.0f, 1.0f, 1.0f);

		// Keep tracing paths until we reach the specified max
		int pathLength;
		const int maxPathLength = params.MaxPathLength;
		for (pathLength = 1; pathLength <= maxPathLength || maxPathLength == -1; ++pathLength)
		{
#if 0
			// See if we should randomly terminate this path using Russian Roullete
			const int rouletteDepth = params.RussianRouletteDepth;
			if (pathLength >= rouletteDepth && rouletteDepth != -1)
			{
				float continueProbability = std::min<float>(params.RussianRouletteProbability, ComputeLuminance(throughput));
				if (randomGenerator.RandomFloat() > continueProbability)
					break;
				throughput /= continueProbability;
				irrThroughput /= continueProbability;
			}
#endif

			// Set this to true to keep the loop going
			bool continueTracing = false;

			// Check for intersection with the scene

			float sceneDistance = FLT_MAX;
			Contact contact;
			if (World::Instance()->GetScene()->RayCheck(contact, ray, params.RayLen, LFX_TERRAIN | LFX_MESH))
			{
				sceneDistance = contact.td;
			}

			Float3 rayOrigin = ray.orig;
			Float3 rayDir = ray.dir;

			if (sceneDistance < FLT_MAX)
			{
				// We hit a triangle in the scene
				if (pathLength == maxPathLength)
				{
					// There's no point in continuing anymore, since none of our scene surfaces are emissive.
					break;
				}

				if (contact.backFacing)
					break;

				Vertex hitSurface = contact.vhit;
				Mat3 tangentToWorld;
				tangentToWorld.SetXBasis(hitSurface.Tangent);
				tangentToWorld.SetYBasis(hitSurface.Binormal);
				tangentToWorld.SetZBasis(hitSurface.Normal);

				Float3 material = ((Material *)contact.mtl)->diffuse * params.DiffuseScale;
				if (1)
				{
					// Compute direct lighting from the sun
					Float3 directLighting;
					Float3 directIrradiance;
					if (1)
					{
						Float3 diffuse = Float3(0, 0, 0);
						Float3 irradiance = Float3(0, 0, 0);

						std::vector<Light *> lights;
						GetLightList(lights, hitSurface.Position);
						
						for (size_t i = 0; i < lights.size(); ++i)
						{
							RT_CalcuLighting(ray.orig, hitSurface, lights[i], diffuse, irradiance);
						}

						directLighting = diffuse;
						directIrradiance = irradiance;
					}

					radiance += directLighting * throughput;
					irradiance += directIrradiance * irrThroughput;
				}

				// Pick a new path, using MIS to sample both our diffuse and specular BRDF's
				if (1)
				{
					// Randomly select if we should sample our diffuse BRDF, or our specular BRDF
					Float2 brdfSample = params.SampleSet->BRDF();
					if (pathLength > 1)
						brdfSample = randomGenerator.RandomFloat2();

					//Float3 v = Float3::Normalize(rayOrigin - hitSurface.Position);

					// We're sampling the diffuse BRDF, so sample a cosine-weighted hemisphere
					Float3 sampleDir;
					sampleDir = ILBaker::SampleCosineHemisphere(brdfSample.x, brdfSample.y);
					sampleDir = Float3::Normalize(Mat3::Transform(sampleDir, tangentToWorld));

					if (Float3::Dot(sampleDir, hitSurface.Normal) > 0.0f)
					{
						throughput = throughput * material;
						irrThroughput = irrThroughput * 3.1415926f;

						// Generate the ray for the new path
						ray.orig = hitSurface.Position + sampleDir * 0.01f;
						ray.dir = sampleDir;

						continueTracing = true;
					}
				}
			}
			else
			{
				hitSky = true;

				radiance += params.SkyRadiance * throughput;
				irradiance += params.SkyRadiance * irrThroughput;
			}

			if (continueTracing == false)
				break;
		}

		illuminance = ComputeLuminance(irradiance);
		return radiance;
	}

	Float4 ILBakerRaytrace::_doLighting(const Vertex & bakePoint, int texelIdxX, int texelIdxY)
	{
		const int groupTexelIdxX = texelIdxX % BakeGroupSizeX;
		const int groupTexelIdxY = texelIdxY % BakeGroupSizeY;
		const int groupTexelIdx = groupTexelIdxX * groupTexelIdxY;
		const int samplesPerTexel = _cfg.SqrtNumSamples * _cfg.SqrtNumSamples;

		const int texelIdx = texelIdxY * _ctx.MapWidth + texelIdxX;
		if (texelIdxX >= _ctx.MapWidth || texelIdxY >= _ctx.MapHeight)
			return Float4(0, 0, 0, 0);

		Mat3 tangentToWorld;
		tangentToWorld.SetXBasis(bakePoint.Tangent);
		tangentToWorld.SetYBasis(bakePoint.Binormal);
		tangentToWorld.SetZBasis(bakePoint.Normal);

		DiffuseBaker baker;
		baker.Init(samplesPerTexel);

		// Loop over all texels in the 8x8 group, and compute 1 sample for each
		for (int sampleIdx = 0; sampleIdx < samplesPerTexel; ++sampleIdx)
		{
			IntegrationSampleSet sampleSet;
			sampleSet.Init(_ctx.Samples, groupTexelIdx, sampleIdx);

			// Create a random ray direction in tangent space, then convert to world space
			Float3 rayStart = bakePoint.Position;
			Float3 rayDirTS = baker.SampleDirection(sampleSet.Pixel());
			Float3 rayDir = Mat3::Transform(rayDirTS, tangentToWorld);
			rayDir = Float3::Normalize(rayDir);

			PathTracerParams params;
			params.SampleSet = &sampleSet;
			params.RayDir = rayDir;
			params.RayStart = rayStart + 0.001f * rayDir;
			params.RayLen = 1000;
			params.MaxPathLength = _cfg.MaxPathLength;
			//params.RussianRouletteDepth = _cfg.RussianRouletteDepth;
			//params.RussianRouletteProbability = 0.5f;
			params.SkyRadiance = _cfg.SkyRadiance;
			params.DiffuseScale = _cfg.DiffuseScale;

			float illuminance = 0.0f;
			bool hitSky = false;
			Float3 sampleResult = PathTrace(params, _ctx.RandomGenerator, illuminance, hitSky);

			baker.AddSample(rayDirTS, sampleIdx, sampleResult, hitSky);
		}

		Float4 texelResults[1];
		baker.FinalResult(texelResults);

		return texelResults[0];
	}

	void ILBakerRaytrace::Run(Rasterizer * rs)
	{
		_ctx.MapWidth = rs->_width;
		_ctx.MapHeight = rs->_height;
		_ctx.BakeOutput.resize(rs->_width * rs->_height);
		for (size_t i = 0; i < _ctx.BakeOutput.size(); ++i)
		{
			_ctx.BakeOutput[i] = Float4(0, 0, 0, 0);
		}

		GenerateIntegrationSamples(_ctx.Samples, _cfg.SqrtNumSamples, BakeGroupSize, 1, 5, _ctx.RandomGenerator);

		int index = 0;
		for (int v = 0; v < _ctx.MapHeight; ++v)
		{
			for (int u = 0; u < _ctx.MapWidth; ++u)
			{
				Float4 color = Float4(0, 0, 0, 0);
				
				const RVertex& bakePoint = rs->_rchart[index];
				if (bakePoint.MaterialId != -1)
				{
					color = _doLighting(bakePoint, u, v);
				}

				_ctx.BakeOutput[index++] = color;
			}
		}
	}

}