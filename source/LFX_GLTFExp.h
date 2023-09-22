#pragma once

#include "LFX_Types.h"

namespace LFX {

	class LFX_ENTRY GLTFExp
	{
	public:
		static bool Export();
	};

#ifndef _WIN32
	inline bool GLTFExp::Export()
	{
		return false;
	}
#endif

}