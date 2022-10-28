#pragma once

#include "LFX_SH.h"
#include "LFX_Entity.h"
#include "LFX_ILPathTrace.h"

namespace LFX {

	struct SHProbe : public Entity
	{
		Float3 position;
		Float3 normal;
		std::vector<Float3> coefficients;

		virtual int GetType() override { return LFX_SHPROBE; }
	};

	class SHBaker
	{
	public:
		struct Context
		{
			int Samples = 1024;
			int MaxPathLength = 0;
			float LightingScale = 0;
			Float3 SkyRadiance;
			ILBaker::Random Random;
		};

		Context _ctx;

	public:
		void Run(SHProbe* probe);
	};

}