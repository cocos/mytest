#include "LFX_Light.h"

namespace LFX {

	void DoLighting(float & kd, float & ka, float & ks, const Vertex & v, Light * pLight)
	{
		switch (pLight->Type)
		{
		case Light::DIRECTION:
		{
			kd = v.Normal.dot(-pLight->Direction);

			kd = Clamp<float>(kd, 0, 1);
			ks = 1;
			ka = 1;
		}
		break;

		case Light::POINT:
		{
			Float3 lightDir = pLight->Position - v.Position;
			float length = lightDir.len();
			lightDir.normalize();

			kd = v.Normal.dot(lightDir);
			ka = (length - pLight->AttenStart) / (pLight->AttenEnd - pLight->AttenStart);

			kd = Clamp<float>(kd, 0, 1);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), pLight->AttenFallOff);
			ks = 1;
		}
		break;

		case Light::SPOT:
		{
			Float3 spotDir = pLight->Position - v.Position;
			float length = spotDir.len();
			spotDir.normalize();

			float kd = v.Normal.dot(-pLight->Direction);
			float ka = (length - pLight->AttenStart) / (pLight->AttenEnd - pLight->AttenStart);
			float ks = (spotDir.dot(pLight->Direction) - pLight->SpotOuter) / (pLight->SpotInner - pLight->SpotOuter);

			kd = Clamp<float>(kd, 0, 1);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), pLight->AttenFallOff);
			ks = std::pow(Clamp<float>(ks, 0, 1), pLight->SpotFallOff);
		}
		break;
		}
	}

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