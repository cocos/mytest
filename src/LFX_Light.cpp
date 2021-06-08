#include "LFX_Light.h"

namespace LFX {

	Float3 ACESToneMap(const Float3& c)
	{
		Float3 color = c;
		if (color.x > 8.0f) {
			color.x = 8.0f;
		}
		if (color.y > 8.0f) {
			color.y = 8.0f;
		}
		if (color.z > 8.0f) {
			color.z = 8.0f;
		}

		const float A = 2.51f;
		const float B = 0.03f;
		const float C = 2.43f;
		const float D = 0.59f;
		const float E = 0.14f;
		return (color * (A * color + B)) / (color * (C * color + D) + E);
	}

	float SmoothDistAtt(float distSqr, float invSqrAttRadius)
	{
		float factor = distSqr * invSqrAttRadius;
		float smoothFactor = Clamp(1.0f - factor * factor, 0.0f, 1.0f);
		return smoothFactor * smoothFactor;
	}

	float GetDistAtt(float distSqr, float invSqrAttRadius)
	{
		float attenuation = 1.0f / std::max(distSqr, 0.01f * 0.01f);
		attenuation *= SmoothDistAtt(distSqr, invSqrAttRadius);
		return attenuation;
	}

#if LFX_VERSION >= 30
	float CalcLightAtten(float distSqr, float radius, float size)
	{
		float litRadius = radius;
		float litRadiusSqr = litRadius * litRadius;
		float illum = Pi * (litRadiusSqr / std::max(litRadiusSqr, distSqr));
		float attRadiusSqrInv = 1.0 / std::max(size, 0.01f);
		attRadiusSqrInv *= attRadiusSqrInv;
		float att = GetDistAtt(distSqr, attRadiusSqrInv);

		return illum * att;
	}
#else
	float CalcLightAtten(float d, float start, float end)
	{
		float ka = (d - start) / (end - start);
		ka = 1 - Clamp<float>(ka, 0, 1);
		return ka;
	}
#endif

	float GetAngleAtt(const Float3& L, const Float3& litDir, float litAngleScale, float litAngleOffset) {
		float cd = litDir.dot(L);
		float attenuation = Clamp(cd * litAngleScale + litAngleOffset, 0.0f, 1.0f);
		return (attenuation * attenuation);
	}

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
#if LFX_VERSION < 30
			float length = lightDir.len();
			ka = (length - pLight->AttenStart) / (pLight->AttenEnd - pLight->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), pLight->AttenFallOff);
#else
			ka = CalcLightAtten(lightDir.lenSqr(), pLight->Radius, pLight->Size);
#endif
			lightDir.normalize();
			kd = v.Normal.dot(lightDir);
			kd = Clamp<float>(kd, 0, 1);

			ks = 1;
		}
		break;

		case Light::SPOT:
		{
			Float3 spotDir = pLight->Position - v.Position;

			kd = v.Normal.dot(-pLight->Direction);
			kd = Clamp<float>(kd, 0, 1);
#if LFX_VERSION < 30
			float length = spotDir.len();
			ka = (length - pLight->AttenStart) / (pLight->AttenEnd - pLight->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), pLight->AttenFallOff);
#else
			ka = CalcLightAtten(spotDir.lenSqr(), pLight->Radius, pLight->Size);
#endif

			spotDir.normalize();
#if LFX_VERSION < 30
			ks = (spotDir.dot(-pLight->Direction) - pLight->SpotOuter) / (pLight->SpotInner - pLight->SpotOuter);
			ks = std::pow(Clamp<float>(ks, 0, 1), pLight->SpotFallOff);
#else
			float cosInner = std::max(spotDir.dot(-pLight->Direction), 0.01f);
			float cosOuter = pLight->SpotOuter;
			float litAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
			float litAngleOffset = -cosOuter * litAngleScale;
			ks = GetAngleAtt(spotDir, -pLight->Direction, litAngleScale, litAngleOffset);
#endif
		}
		break;
		}

		kd /= Pi;
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