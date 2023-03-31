#include "LFX_Light.h"
#include "LFX_World.h"

namespace LFX {

	bool IsLightVisible(Light * pLight, const Aabb & bound)
	{
		if (pLight->Type == Light::POINT || pLight->Type == Light::SPOT) {
			Float3 lightExtend = Float3(pLight->AttenEnd, pLight->AttenEnd, pLight->AttenEnd);

			Aabb lightBounds;
			lightBounds.minimum = pLight->Position - lightExtend;
			lightBounds.maximum = pLight->Position + lightExtend;
			return lightBounds.Intersect(bound);
		}

		return true;
	}

	bool IsLightVisible(Light * pLight, const Float3 & point)
	{
		if (pLight->Type == Light::POINT || pLight->Type == Light::SPOT) {
			Float3 lightExtend = Float3(pLight->AttenEnd, pLight->AttenEnd, pLight->AttenEnd);

			Aabb lightBounds;
			lightBounds.minimum = pLight->Position - lightExtend;
			lightBounds.maximum = pLight->Position + lightExtend;
			return lightBounds.Contain(point);
		}

		return true;
	}

	float CalcShadowMask(const Float3& pos, Light* pLight, int queryFlags)
	{
		float len = 0;
		Ray ray;

		if (pLight->Type != Light::DIRECTION) {
			ray.dir = pLight->Position - pos;
			len = ray.dir.len();
			ray.dir.normalize();
		}
		else {
			ray.dir = -pLight->Direction;
			len = FLT_MAX;
		}

		ray.orig = pos + ray.dir * UNIT_LEN * 0.01f;

		if (len > 0.01f * UNIT_LEN) {
			if (World::Instance()->GetScene()->Occluded(ray, len, queryFlags)) {
				return pLight->ShadowMask;
			}
		}

		return 1.0f;
	}

}