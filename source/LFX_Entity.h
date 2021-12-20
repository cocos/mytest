#pragma once

#include "LFX_Math.h"
#include "LFX_Geom.h"
#include "LFX_Texture.h"

namespace LFX {

#define LFX_MESH 0x01
#define LFX_TERRAIN 0x02

	struct LFX_ENTRY Material
	{
		Float3 diffuse;
		Texture* texture;

		Material()
		{
			diffuse = Float3(1, 1, 1);
			texture = NULL;
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