#pragma once

#include "LFX_Math.h"
#include "LFX_Geom.h"

namespace LFX {

	struct LFX_ENTRY Light
	{
		enum {
			POINT,
			SPOT,
			DIRECTION,
		};

		int Type;

		Float3 Position;
		Float3 Direction;
		Float3 Color;

		float AttenStart;
		float AttenEnd;
		float AttenFallOff;

		float SpotInner;
		float SpotOuter;
		float SpotFallOff;

		float DirectScale;
		float IndirectScale;
		bool GIEnable;
		bool CastShadow;

		Light()
		{
			Type = Light::POINT;

			Position = Float3(0, 0, 0);
			Direction = Float3(0, 0, 1);

			Color = Float3(0, 0, 0);

			AttenStart = 0;
			AttenEnd = 1;
			AttenFallOff = 1;

			SpotInner = 1;
			SpotOuter = 0.7071f;
			SpotFallOff = 1;

			DirectScale = 1;
			IndirectScale = 1;
			CastShadow = false;
			GIEnable = false;
		}
	};

	LFX_ENTRY void DoLighting(float & kd, float & ka, float & ks, const Vertex & v, Light * pLight);
	LFX_ENTRY bool IsLightVisible(Light * pLight, const Aabb & bound);
	LFX_ENTRY bool IsLightVisible(Light * pLight, const Float3 & point);
}