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
		Texture* DiffuseMap;
		Texture* MetallicMap;
		Texture* RoughnessMap;
		Texture* PBRMap;

		Material()
		{
			Metallic = 0;
			Roughness = 0;
			Diffuse = Float3(1, 1, 1);
			DiffuseMap = NULL;
			MetallicMap = NULL;
			RoughnessMap = NULL;
			PBRMap = NULL;
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