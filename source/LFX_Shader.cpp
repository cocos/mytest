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
		//float attenuation = 1.0f / std::max(distSqr, 0.1f);
		attenuation *= SmoothDistAtt(distSqr, invSqrAttRadius);
		return attenuation;
	}

#if LFX_VERSION >= 30
	float CalcLightAtten(float distSqr, float size, float range)
	{
		float litRadius = size;
		float litRadiusSqr = litRadius * litRadius;
#if LFX_VERSION >= 35
		float illum = (litRadiusSqr / std::max(litRadiusSqr, distSqr));
#else
		float illum = Pi * (litRadiusSqr / std::max(litRadiusSqr, distSqr));
#endif
		float attRadiusSqrInv = 1.0f / std::max(range, 0.01f);
		attRadiusSqrInv *= attRadiusSqrInv;
		float att = GetDistAtt(distSqr, attRadiusSqrInv);
		float ka = illum * att;

#if 0
		// Sanity check
		if (ka > 800.0f) {
			int iii = 0;
			float ca = GetDistAtt(distSqr, attRadiusSqrInv);
		}
#endif

		return ka;
	}
#else
	float CalcLightAtten(float d, float start, float end)
	{
		float ka = (d - start) / (end - start);
		ka = 1 - Clamp<float>(ka, 0, 1);
		return ka;
	}
#endif

	float GetAngleAtt(const Float3& L, const Float3& litDir, float litAngleScale, float litAngleOffset)
	{
		float cd = litDir.dot(L);
		float att = Clamp(cd * litAngleScale + litAngleOffset, 0.0f, 1.0f);
		return (att * att);
	}

	float GetRoughnessContributes(float roughness, float metallic)
	{
		return std::pow(roughness * metallic, 1.5f);
	}

	float GGXMobile(float roughness, float NoH, Float3 H, Float3 N)
	{
		Float3 NxH = Float3::Cross(N, H);
		float OneMinusNoHSqr = Float3::Dot(NxH, NxH);
		float a = roughness * roughness;
		float n = NoH * a;
		float p = a / (OneMinusNoHSqr + n * n);
		return p * p;
	}

	Float3 BRDFApprox(Float3 specular, float roughness, float NoV)
	{
		const Float4 c0 = Float4(-1.0f, -0.0275f, -0.572f, 0.022f);
		const Float4 c1 = Float4(1.0f, 0.0425f, 1.04f, -0.04f);
		Float4 r = roughness * c0 + c1;
		float a004 = std::min(r.x * r.x, exp2(-9.28f * NoV)) * r.x + r.y;
		Float2 AB = Float2(-1.04f, 1.04f) * a004 + Float2(r.z, r.w);
		AB.y *= Clamp(50.0f * specular.y, 0.0f, 1.0f);
		return specular * AB.x + AB.y;
	}

	float CalcSpecular(float roughness, float NoH, Float3 H, Float3 N)
	{
		return (roughness * 0.25f + 0.25f) * GGXMobile(roughness, NoH, H, N);
	}

	void Shader::DoLighting(Float3& color, float& kl, 
		const Float3& eye, const Vertex& vertex, const Light* light,
		const Material* material, bool sampler, bool specular)
	{
		float kd = 0, ks = 0, ka = 0;
		const Float3 E = eye;
		const Float3 P = vertex.Position;
		const Float3 N = vertex.Normal;
		const Float2 UV = vertex.UV;
		Float3 L;

		switch (light->Type) {
		case Light::DIRECTION: {
			L = -light->Direction;

			kd = N.dot(L);
			kd = Clamp<float>(kd, 0, 1);

			ks = 1;

			ka = 1;
		}
		break;

		case Light::POINT: {
			L = light->Position - P;
#if LFX_VERSION < 30
			float length = L.len();
			ka = (length - light->AttenStart) / (light->AttenEnd - light->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), light->AttenFallOff);
#else
			ka = CalcLightAtten(L.lenSqr(), light->Size, light->Range);
#endif
			L.normalize();
			kd = vertex.Normal.dot(L);
			kd = Clamp<float>(kd, 0, 1);

			ks = 1;
		}
		break;

		case Light::SPOT: {
			L = -light->Direction;

			kd = vertex.Normal.dot(L);
			kd = Clamp<float>(kd, 0, 1);

			Float3 spotDir = light->Position - P;
#if LFX_VERSION < 30
			float length = spotDir.len();
			ka = (length - pLight->AttenStart) / (light->AttenEnd - light->AttenStart);
			ka = std::pow(1 - Clamp<float>(ka, 0, 1), light->AttenFallOff);
#else
			ka = CalcLightAtten(spotDir.lenSqr(), light->Size, light->Range);
#endif

			spotDir.normalize();
#if LFX_VERSION < 30
			ks = (spotDir.dot(L) - light->SpotOuter) / (light->SpotInner - light->SpotOuter);
			ks = std::pow(Clamp<float>(ks, 0, 1), light->SpotFallOff);
#else
			float cosInner = std::max(spotDir.dot(L), 0.01f);
			float cosOuter = light->SpotOuter;
			float litAngleScale = 1.0f / std::max(0.001f, cosInner - cosOuter);
			float litAngleOffset = -cosOuter * litAngleScale;
			ks = GetAngleAtt(spotDir, L, litAngleScale, litAngleOffset);
#endif
		}
		break;
		}

		kl = kd * ks * ka;
		if (kl > 0) {
			Float3 diffuse = material->Diffuse;
			if (sampler && material->DiffuseMap != nullptr) {
				Float4 textureColor(1, 1, 1, 1);
				textureColor = material->DiffuseMap->SampleColor(UV.x, UV.y, true);
				diffuse.x *= textureColor.x;
				diffuse.y *= textureColor.y;
				diffuse.z *= textureColor.z;
			}

			color = kl * diffuse / Pi * light->Color;
			 
#if 0
			if (specular) {
				const float metallic = material->GetSurfaceMetallic(UV.x, UV.y);
				const float roughness = material->GetSurfaceRoughness(UV.x, UV.y);
				const float roughnessContributes = std::pow(roughness * metallic, 1.5f);
				const float s = Saturate(roughnessContributes + (1.0f - metallic));
				
				color *= s;

				Float3 V = Float3::Normalize(E - P);
				Float3 H = Float3::Normalize(L + V);
				float NL = std::max(std::abs(Float3::Dot(N, L)), 0.0f);
				float NV = std::max(std::abs(Float3::Dot(N, V)), 0.0f);
				float NH = std::max(Float3::Dot(N, H), 0.0f);

				Float3 F0 = Float3(0.04f, 0.04f, 0.04f);
				Float3 specularColor = Float3::Lerp(F0, diffuse, metallic);
				specularColor = BRDFApprox(specularColor, roughness, NV);
				specularColor *= CalcSpecular(roughness, NH, H, N);

				color += ks * ka * specularColor;
			}
#endif
		}
		else {
			color = Float3(0, 0, 0);
		}
	}

}