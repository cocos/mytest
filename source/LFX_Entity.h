#pragma once

#include "LFX_Math.h"
#include "LFX_Geom.h"
#include "LFX_Texture.h"

namespace LFX {

#define LFX_MESH 0x01
#define LFX_TERRAIN 0x02
#define LFX_SHPROBE 0x03

	struct LFX_ENTRY Material
	{
		float Metallic;
		float Roughness;
		Float3 Diffuse;
		Float3 Emissive;
		Texture* EmissiveMap;
		Texture* DiffuseMap;
		Texture* MetallicMap;
		Texture* RoughnessMap;
		Texture* PBRMap;

		Material()
		{
			Metallic = 0;
			Roughness = 0;
			Diffuse = Float3(1, 1, 1);
			Emissive = Float3(0, 0, 0);
			EmissiveMap = NULL;
			DiffuseMap = NULL;
			MetallicMap = NULL;
			RoughnessMap = NULL;
			PBRMap = NULL;
		}

		Float3 GetSurfaceEmissive(float u, float v) const
		{
			Float3 value = Emissive;

			if (EmissiveMap != nullptr) {
				value = value * EmissiveMap->SampleColor3(u, v);
			}

			return value;
		}

		float GetSurfaceMetallic(float u, float v) const
		{
			float value = Metallic;

			float m = 1.0f;
			if (PBRMap != nullptr) {
				m = PBRMap->SampleColor(u, v).z;
			}
			else if (MetallicMap != nullptr) {
				m = MetallicMap->SampleColor(u, v).x;
			}

			return value * m;
		}

		float GetSurfaceRoughness(float u, float v) const
		{
			float value = Roughness;

			float m = 1.0f;
			if (PBRMap != nullptr) {
				m = PBRMap->SampleColor(u, v).y;
			}
			else if (RoughnessMap != nullptr) {
				m = RoughnessMap->SampleColor(u, v).x;
			}

			return value * m;
		}
	};

	class LFX_ENTRY Entity
	{
	public:
		Entity();
		virtual ~Entity();

		virtual int GetType() = 0;

		void SetUserData(void * data, int i = 0);
		void * GetUserData(int i);

	protected:
		void * mUserData[8];
	};

}