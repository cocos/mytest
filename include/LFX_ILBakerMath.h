#pragma once

#include <limits>
#include "LFX_Math.h"

namespace LFX { namespace ILBaker {

	// Clamps a value to [0, 1]
	template<typename T> T Saturate(T val)
	{
		return Clamp<T>(val, T(0.0f), T(1.0f));
	}

	inline Float3 Saturate(Float3 val)
	{
		Float3 result;
		result.x = Clamp<float>(val.x, 0.0f, 1.0f);
		result.y = Clamp<float>(val.y, 0.0f, 1.0f);
		result.z = Clamp<float>(val.z, 0.0f, 1.0f);
		return result;
	}

	// Returns the fractional part of x
	inline float Frac(float x)
	{
		float intPart;
		return std::modf(x, &intPart);
	}

	// Returns the fractional part of x
	inline Float2 Frac(Float2 x)
	{
		return Float2(Frac(x.x), Frac(x.y));
	}

	// Smoothstep cubic interpolation
	inline float Smoothstep(float start, float end, float x)
	{
		x = Saturate((x - start) / (end - start));
		return x * x * (3.0f - 2.0f * x);
	}

	// linear -> sRGB conversion
	inline Float3 LinearTosRGB(Float3 color)
	{
		Float3 x = color * 12.92f;
		Float3 y = 1.055f * Pow(color, 1.0f / 2.4f) - 0.055f * Float3(1, 1, 1);

		Float3 clr = color;
		clr.x = color.x < 0.0031308f ? x.x : y.x;
		clr.y = color.y < 0.0031308f ? x.y : y.y;
		clr.z = color.z < 0.0031308f ? x.z : y.z;

		return clr;
	}

	// sRGB -> linear conversion
	inline Float3 SRGBToLinear(Float3 color)
	{
		Float3 x = color / 12.92f;
		Float3 y = Pow((color + 0.055f * Float3(1, 1, 1)) / 1.055f, 2.4f);

		Float3 clr = color;
		clr.x = color.x <= 0.04045f ? x.x : y.x;
		clr.y = color.y <= 0.04045f ? x.y : y.y;
		clr.z = color.z <= 0.04045f ? x.z : y.z;

		return clr;
	}

	inline float ComputeLuminance(Float3 color)
	{
		return Float3::Dot(color, Float3(0.2126f, 0.7152f, 0.0722f));
	}

	// Convert from spherical coordinates to Cartesian coordinates(x, y, z)
	// Theta represents how far away from the zenith (north pole/+Y) and phi represents how far
	// away from the 'right' axis (+X).
	inline void SphericalToCartesianXYZYUP(float r, float theta, float phi, Float3& xyz)
	{
		xyz.x = r * std::cos(phi) * std::sin(theta);
		xyz.y = r * std::cos(theta);
		xyz.z = r * std::sin(theta) * std::sin(phi);
	}

}}