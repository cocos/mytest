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
			const Float3& eye, const Vertex& vertex, const Light* light,
			const Material* material, bool sampler, bool specular);
	};

}