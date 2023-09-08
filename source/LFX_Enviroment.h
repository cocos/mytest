#pragma once

#include "LFX_Types.h"
#include "LFX_Math.h"

namespace LFX {

	struct Environment
	{
		Float3 SkyColor;
		Float3 GroundColor;
		float SkyIllum;

		Environment()
		{
			SkyColor = Float3(0, 0, 0);
			GroundColor = Float3(0, 0, 0);
			SkyIllum = 1.0f;
		}
	};

}