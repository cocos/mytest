#pragma once

#include "LFX_Math.h"
#include "LFX_Geom.h"
#include "LFX_Texture.h"

namespace LFX {

	struct LFX_ENTRY LightmapValue
	{
		Float3 Diffuse;
		float Shadow;
		float AO;

		LightmapValue()
			: Diffuse(0, 0, 0)
			, Shadow(1)
			, AO(1)
		{
		}
	};

	struct LFX_ENTRY Light
	{
		enum {
			POINT,
			SPOT,
			DIRECTION,
		};

		String Name;
		int Type;
		Float3 Position;
		Float3 Direction;
		Float3 Color;

		union {
			struct
			{
				float AttenStart;
				float AttenEnd;
				float AttenFallOff;
			};

			struct
			{
				float Size;
				float Range;
				float Luminance;
			};
		};

		float SpotInner;
		float SpotOuter;
		float SpotFallOff;

		float DirectScale;
		float IndirectScale;
		bool GIEnable;
		bool CastShadow;
		bool SaveShadowMask;
		float ShadowMask;

		Float4 transform[4];

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
			GIEnable = true;
			CastShadow = false;
			SaveShadowMask = false;
			ShadowMask = 0.0f;

			// identity
			transform[0] = Float4(1, 0, 0, 0);
			transform[1] = Float4(0, 1, 0, 0);
			transform[2] = Float4(0, 0, 1, 0);
			transform[3] = Float4(0, 0, 0, 1);
		}
	};

	LFX_ENTRY bool IsLightVisible(Light* pLight, const Aabb& bound);
	LFX_ENTRY bool IsLightVisible(Light* pLight, const Float3& point);
	LFX_ENTRY float CalcShadowMask(const Float3& pos, Light* pLight, int queryFlags);

	struct LFX_ENTRY SkyLight
	{
		Float3 SkyColor;
		Float3 GroundColor;
		float illum;
		Texture* CubeMap[6];

		SkyLight()
		{
			SkyColor = Float3(0, 0, 0);
			GroundColor = Float3(0, 0, 0);
			illum = 1.0f;

			for (int i = 0; i < 6; ++i) {
				CubeMap[i] = nullptr;
			}
		}
	};
}