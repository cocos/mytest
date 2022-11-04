#pragma once

#include "LFX_Types.h"
#include "LFX_Math.h"
#include "LFX_Geom.h"
#include "LFX_Light.h"
#include "LFX_Entity.h"

namespace LFX {

	class LFX_ENTRY Shader
	{
	public:
		virtual ~Shader() {}

		static Float3 ACESToneMap(const Float3& color);

		virtual void DoLighting(Float3& color, float& kl, 
			const Vertex& v, const Light* light,
			const Material* mtl, bool textureSampler, bool specular);
	};

}