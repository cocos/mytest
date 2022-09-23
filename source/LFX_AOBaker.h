/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "LFX_Types.h"
#include "LFX_Entity.h"
#include "LFX_ILBakerRandom.h"
#include "LFX_ILBakerSampling.h"

namespace LFX {

	class AOBaker
	{
	public:
		AOBaker();
		~AOBaker();

		//Float3 Calc(const RVertex& v, int flags, void* entity);

		Float3 Calc(const Vertex& v, int flags, void* entity);

	protected:
		Float4 hd[29];
		Float4 ld[13];
		ILBaker::Random Random;
	};

}

