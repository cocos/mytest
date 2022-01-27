#include "LFX_Light.h"

namespace LFX {

	bool IsLightVisible(Light * pLight, const Aabb & bound)
	{
		if (pLight->Type == Light::POINT || pLight->Type == Light::SPOT)
		{
			Float3 lightExtend = Float3(pLight->AttenEnd, pLight->AttenEnd, pLight->AttenEnd);

			Aabb lightBound;
			lightBound.minimum = pLight->Position - lightExtend;
			lightBound.maximum = pLight->Position + lightExtend;

			return lightBound.Intersect(bound);
		}

		return true;
	}

	bool IsLightVisible(Light * pLight, const Float3 & point)
	{
		if (pLight->Type == Light::POINT || pLight->Type == Light::SPOT)
		{
			Float3 lightExtend = Float3(pLight->AttenEnd, pLight->AttenEnd, pLight->AttenEnd);

			Aabb lightBound;
			lightBound.minimum = pLight->Position - lightExtend;
			lightBound.maximum = pLight->Position + lightExtend;

			return lightBound.Contain(point);
		}

		return true;
	}

}