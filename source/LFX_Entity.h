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
		float AlphaCutoff;
		float Metallic;
		float Roughness;
		float GIWeight;
		Float3 Diffuse;
		Float3 Emissive;
		Texture* EmissiveMap;
		Texture* DiffuseMap;
		Texture* MetallicMap;
		Texture* RoughnessMap;
		Texture* PBRMap;
		Float4 TilingOffset;

		String _emissiveMapFile;
		String _diffuseMapFile;
		String _normalMapFile;
		String _pbrMapFile;

		Material()
		{
			AlphaCutoff = 0.5f;
			Metallic = 0;
			Roughness = 0;
			GIWeight = 0.5f;
			Diffuse = Float3(1, 1, 1);
			Emissive = Float3(0, 0, 0);
			EmissiveMap = NULL;
			DiffuseMap = NULL;
			MetallicMap = NULL;
			RoughnessMap = NULL;
			PBRMap = NULL;
			TilingOffset = Float4(1, 1, 0, 0);
		}

		Float3 GetSurfaceEmissive(float u, float v) const
		{
			Float3 value = Emissive;

			u = u * TilingOffset.x + TilingOffset.z;
			v = v * TilingOffset.y + TilingOffset.w;

			if (EmissiveMap != nullptr) {
				value = value * EmissiveMap->SampleColor3(u, v);
			}

			return value;
		}

		Float3 GetSurfaceDiffuse(float u, float v) const
		{
			Float3 value = Diffuse;

			u = u * TilingOffset.x + TilingOffset.z;
			v = v * TilingOffset.y + TilingOffset.w;

			if (DiffuseMap != nullptr) {
				value = value * DiffuseMap->SampleColor3(u, v);
			}

			return value;
		}

		float GetSurfaceMetallic(float u, float v) const
		{
			float value = Metallic;

			u = u * TilingOffset.x + TilingOffset.z;
			v = v * TilingOffset.y + TilingOffset.w;

			float m = 1.0f;
			if (PBRMap != nullptr) {

				m = PBRMap->SampleColor(u, v).x;
			}
			else if (MetallicMap != nullptr) {
				m = MetallicMap->SampleColor(u, v).x;
			}

			return value * m;
		}

		float GetSurfaceRoughness(float u, float v) const
		{
			float value = Roughness;

			u = u * TilingOffset.x + TilingOffset.z;
			v = v * TilingOffset.y + TilingOffset.w;

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