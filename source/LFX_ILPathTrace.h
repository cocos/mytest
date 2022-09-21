/*
* Slay Engine
* Author: SiZhong.Wang(M-001)
* Copyright: Silvereye Information, Inc. All Rights Reserved.
*/
#pragma once

#include "LFX_Geom.h"
#include "LFX_Entity.h"
#include "LFX_ILBakerMath.h"
#include "LFX_ILBakerSamples.h"

namespace LFX { namespace ILBaker {

	#define DEFAULT_RAYTRACE_MAX_LENGHT 1000.0f 

	struct EIntegrationTypes
	{
		enum
		{
			Pixel = 0,
			Lens,
			BRDF,
			Sun,
			AreaLight,

			NumValues,
		};
	};

	struct IntegrationSampleSet
	{
		std::array<Float2, EIntegrationTypes::NumValues> Samples;

		void Init(const ILBaker::IntegrationSamples& samples, int pixelIdx, int sampleIdx)
		{
			samples.GetSampleSet(pixelIdx, sampleIdx, Samples.data());
		}

		Float2 Pixel() const { return Samples[EIntegrationTypes::Pixel]; }
		Float2 Lens() const { return Samples[EIntegrationTypes::Lens]; }
		Float2 BRDF() const { return Samples[EIntegrationTypes::BRDF]; }
		Float2 Sun() const { return Samples[EIntegrationTypes::Sun]; }
		Float2 AreaLight() const { return Samples[EIntegrationTypes::AreaLight]; }
	};

	struct PathTraceParams
	{
		const IntegrationSampleSet* sampleSet = nullptr;
		Entity* entity = nullptr;
		Float3 rayStart;
		Float3 rayDir;
		float rayLen = 0.0f;
		int maxPathLength = -1;
		int russianRouletteDepth = 4;
		float russianRouletteProbability = 0.5f;

		float diffuseScale;
		Float3 skyRadiance;
	};

	struct PathTraceResult
	{
		int pathLen = 1;
		Float3 color = Float3(0, 0, 0);
	};

	typedef bool (*PathTraceFunc)(Float3& diffuse, const PathTraceParams& params, const Vertex& vtx, const Material* mtl, bool hitSky);

	PathTraceResult PathTrace(const PathTraceParams& params, PathTraceFunc func, Random& rand, bool& hitSky);

}}