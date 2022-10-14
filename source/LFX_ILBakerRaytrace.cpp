#include "LFX_World.h"
#include "LFX_ILBakerRaytrace.h"
#include "LFX_ILPathTrace.h"
#include "LFX_EmbreeScene.h"

namespace LFX {

	using namespace ILBaker;

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

	//
	void RTCalcuLighting(Float3& diffuse, const Vertex & V, Light * L, const Material * M)
	{
		float kl = 0;
		Float3 color;
		World::Instance()->GetShader()->DoLighting(color, kl, V, L, M, true);
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

		diffuse += kl > 0 ? color * L->IndirectScale : Float3(0, 0, 0);
	}

	void RTGetLightList(std::vector<Light*>& lights, const Float3& point)
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

	bool RTPathTraceFunc(Float3& result, const PathTraceParams& params, const Vertex& vtx, const Material* mtl, bool hitSky)
	{
		if (hitSky) {
			result += params.skyRadiance;
			return true;
		}

		std::vector<Light*> lights;
		RTGetLightList(lights, vtx.Position);
		for (size_t i = 0; i < lights.size(); ++i) {
			Float3 diffuse;
			RTCalcuLighting(diffuse, vtx, lights[i], mtl);
			result += diffuse * params.diffuseScale;
		}

		return true;
	}

	Float4 ILBakerRaytrace::_doLighting(const Vertex& bakePoint, int texelIdxX, int texelIdxY)
	{
		const int groupTexelIdxX = texelIdxX % BakeGroupSizeX;
		const int groupTexelIdxY = texelIdxY % BakeGroupSizeY;
		const int groupTexelIdx = groupTexelIdxX * groupTexelIdxY;
		const int samplesPerTexel = _cfg.SqrtNumSamples * _cfg.SqrtNumSamples;

		const int texelIdx = texelIdxY * _ctx.MapWidth + texelIdxX;
		if (texelIdxX >= _ctx.MapWidth || texelIdxY >= _ctx.MapHeight) {
			return Float4(0, 0, 0, 0);
		}

		Mat3 tangentToWorld;
		tangentToWorld.SetXBasis(bakePoint.Tangent);
		tangentToWorld.SetYBasis(bakePoint.Binormal);
		tangentToWorld.SetZBasis(bakePoint.Normal);

		DiffuseBaker baker;
		baker.Init(samplesPerTexel);

		// Loop over all texels in the 8x8 group, and compute 1 sample for each
		for (int sampleIdx = 0; sampleIdx < samplesPerTexel; ++sampleIdx)
		{
			Random& rand = _ctx.Random;
			IntegrationSampleSet sampleSet;
			sampleSet.Init(_ctx.Samples, groupTexelIdx, sampleIdx);

			// Create a random ray direction in tangent space, then convert to world space
			Float3 rayStart = bakePoint.Position;
			//Float3 rayDirTS = SampleCosineHemisphere(sampleSet.Pixel());
			Float3 rayDirTS = SampleCosineHemisphere(rand.RandomFloat2());
			Float3 rayDir = Mat3::Transform(rayDirTS, tangentToWorld);
			rayDir = Float3::Normalize(rayDir);

			PathTraceParams params;
			params.entity = _ctx.entity;
			params.sampleSet = &sampleSet;
			params.rayDir = rayDir;
			params.rayStart = rayStart + 0.001f * rayDir;
			params.rayLen = DEFAULT_RAYTRACE_MAX_LENGHT;
			params.maxPathLength = _cfg.MaxPathLength;
			//params.russianRouletteDepth = _cfg.RussianRouletteDepth;
			//params.russianRouletteProbability = 0.5f;
			params.skyRadiance = _cfg.SkyRadiance;
			params.diffuseScale = _cfg.DiffuseScale;

			bool hitSky = false;
			PathTraceResult sampleResult = PathTrace(params, RTPathTraceFunc, rand, hitSky);
			baker.AddSample(rayDirTS, sampleIdx, sampleResult.color, hitSky);
		}

		Float4 texelResults[1];
		baker.FinalResult(texelResults);
		return texelResults[0];
	}

	void ILBakerRaytrace::Run(Entity* entity, int w, int h, const std::vector<RVertex>& rchart)
	{
		_cfg.MaxPathLength = 2;

		_ctx.entity = entity;
		_ctx.MapWidth = w;
		_ctx.MapHeight = h;
		_ctx.BakeOutput.resize(w * h);
		for (size_t i = 0; i < _ctx.BakeOutput.size(); ++i) {
			_ctx.BakeOutput[i] = Float4(0, 0, 0, 0);
		}

		GenerateIntegrationSamples(_ctx.Samples, _cfg.SqrtNumSamples, BakeGroupSize, 1, 5, _ctx.Random);

		int index = 0;
		for (int v = 0; v < _ctx.MapHeight; ++v)
		{
			for (int u = 0; u < _ctx.MapWidth; ++u)
			{
				Float4 color = Float4(0, 0, 0, 0);
				
				const RVertex& bakePoint = rchart[index];
				if (bakePoint.MaterialId != -1)
				{
					color = _doLighting(bakePoint, u, v);
				}

				_ctx.BakeOutput[index++] = color;
			}
		}
	}

}