/*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "LFX_Types.h"
#include "LFX_Entity.h"
#include "LFX_Rasterizer.h"

namespace LFX {

	class AOBaker
	{
	public:
		AOBaker();
		~AOBaker();

		Float3 Calcu(const RVertex & v, int flags, void * entity);

	protected:
		Float4 hd[29];
		Float4 ld[13];
	};

}

