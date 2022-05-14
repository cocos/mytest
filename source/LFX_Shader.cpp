#include "LFX_Shader.h"

namespace LFX {
	
	Float3 Shader::ACESToneMap(const Float3& color)
	{
		Float3 c = color;
		if (c.x > 8.0f) {
			c.x = 8.0f;
		}
		if (c.y > 8.0f) {
			c.y = 8.0f;
		}
		if (color.z > 8.0f) {
			c.z = 8.0f;
		}

		const float A = 2.51f;
		const float B = 0.03f;
		const float C = 2.43f;
		const float D = 0.59f;
		const float E = 0.14f;
		return (c * (A * c + B)) / (c * (C * c + D) + E);
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
#if LFX_VERSION >= 35
		float illum = (litRadiusSqr / std::max(litRadiusSqr, distSqr));
#else
		float illum = Pi * (litRadiusSqr / std::max(litRadiusSqr, distSqr));
#endif
		float attRadiusSqrInv = 1.0f / std::max(size, 0.01f);
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

	void Shader::DoLighting(Float3& color, float& kl, const Vertex& v, const Light* light, const Material* mtl)
	{
		float kd = 0, ks = 0, ka = 0;

		switch (light->Type)
		{
		case Light::DIRECTION:
		{
			kd = v.Normal.dot(-light->Direction);
			kd = Clamp<float>(kd, 0, 1);

			ks = 1;

			ka = 1;
		}
		break;

		case Light::POINT:
		{
			Float3 lightDir = light->Position - v.Position;
#if LFX_VERSION < 30
			float length = lightDir.len();
			ka = (length - light->AttenStart) / (light->AttenEnd - light->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), light->AttenFallOff);
#else
			ka = CalcLightAtten(lightDir.lenSqr(), light->Radius, light->Size);
#endif
			lightDir.normalize();
			kd = v.Normal.dot(lightDir);
			kd = Clamp<float>(kd, 0, 1);

			ks = 1;
		}
		break;

		case Light::SPOT:
		{
			Float3 spotDir = light->Position - v.Position;

			kd = v.Normal.dot(-light->Direction);
			kd = Clamp<float>(kd, 0, 1);
#if LFX_VERSION < 30
			float length = spotDir.len();
			ka = (length - pLight->AttenStart) / (light->AttenEnd - light->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), light->AttenFallOff);
#else
			ka = CalcLightAtten(spotDir.lenSqr(), light->Radius, light->Size);
#endif

			spotDir.normalize();
#if LFX_VERSION < 30
			ks = (spotDir.dot(-pLight->Direction) - light->SpotOuter) / (light->SpotInner - light->SpotOuter);
			ks = std::pow(Clamp<float>(ks, 0, 1), light->SpotFallOff);
#else
			float cosInner = std::max(spotDir.dot(-light->Direction), 0.01f);
			float cosOuter = light->SpotOuter;
			float litAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
			float litAngleOffset = -cosOuter * litAngleScale;
			ks = GetAngleAtt(spotDir, -light->Direction, litAngleScale, litAngleOffset);
#endif
		}
		break;
		}

		kl = kd * ks * ka;
		color = kl * mtl->Diffuse / Pi * light->Color;
	}

}