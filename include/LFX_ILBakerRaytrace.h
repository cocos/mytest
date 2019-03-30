#pragma once

#include "LFX_ILBaker.h"
#include "LFX_ILBakerRandom.h"
#include "LFX_ILBakerSampling.h"
#include "LFX_ILBakerSamples.h"

namespace LFX {

	class ILBakerRaytrace
	{
	public:
		static const int TileSize = 16;
		static const int BakeGroupSizeX = 8;
		static const int BakeGroupSizeY = 8;
		static const int BakeGroupSize = BakeGroupSizeX * BakeGroupSizeY;
		static const int NumIntegrationTypes = 5;

	public:
		struct Config
		{
			Float3 SkyRadiance;
			float DiffuseScale; // 光照缩放
			int SqrtNumSamples; // 采样次数 = SqrtNumSamples * SqrtNumSamples
			int MaxPathLength; // 最大Trace次数 
			int RussianRouletteDepth; // 最大Trace深度

			Config()
			{
				SkyRadiance = Float3(0.2f, 0.5f, 0.8f);
				DiffuseScale = 0.5f;
				SqrtNumSamples = 25;
				MaxPathLength = -1;
				RussianRouletteDepth = 4;
			}
		};

		struct Context
		{
			int MapWidth = 0;
			int MapHeight = 0;
			ILBaker::Random RandomGenerator;
			ILBaker::IntegrationSamples Samples;
			std::vector<Float4> BakeOutput;
		};

		Config _cfg;
		Context _ctx;

	public:
		void 
			Run(Rasterizer * rasterizer);

	protected:
		Float4 _doLighting(const Vertex & bakerPoint, int texelIdxX, int texelIdxY);
	};

}