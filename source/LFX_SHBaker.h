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
		struct Config
		{
			Float3 SkyRadiance;
			float DiffuseScale; // π‚’’Àı∑≈

			Config()
			{
				SkyRadiance = Float3(0.2f, 0.5f, 0.8f);
				DiffuseScale = 0.5f;
			}
		};

		Config _cfg;

	public:
		void Run(SHProbe* probe);
	};

}