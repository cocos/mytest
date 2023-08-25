#pragma once

#include "LFX_Math.h"

namespace LFX {

	struct LFX_ENTRY Camera
	{
		String Name;
		float fov;
		float zn, zf;
		Float4 transform[4];

		Camera()
		{
			fov = 60.0;
			zn = 0.1f;
			zf = 1000.0f;
			transform[0] = Float4(1, 0, 0, 0);
			transform[1] = Float4(0, 1, 0, 0);
			transform[2] = Float4(0, 0, 1, 0);
			transform[3] = Float4(0, 0, 0, 1);
		}
	};

}