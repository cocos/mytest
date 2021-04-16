#pragma once

#include "LFX_Vec3.h"
#include "LFX_Vec4.h"

namespace LFX {

	struct RGBA
	{
		union {
			struct {
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
			};

			unsigned char m[4];
			int _value;
		};

		RGBA() { r = 0, g = 0, b = 0, a = 255; }
		RGBA(int c) { _value = c; }
		RGBA(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255) { r = _r, g = _g, b = _b, a = _a; }
		RGBA(const RGBA & c) { _value = c._value; }

		RGBA & operator =(const RGBA & c)
		{
			r = c.r;
			g = c.g;
			b = c.b;
			a = c.a;

			return *this;
		}

		bool operator ==(const RGBA & c) const
		{
			return _value == c._value;
		}

		bool operator !=(const RGBA & c) const
		{
			return _value != c._value;
		}

		void FromFloat4(const Float4 & color)
		{
			r = (unsigned char)(color.x * 255);
			g = (unsigned char)(color.y * 255);
			b = (unsigned char)(color.z * 255);
			a = (unsigned char)(color.w * 255);
		}

		Float4 ToFloat4() const
		{
			Float4 v;

			v.x = r / 255.0f;
			v.y = g / 255.0f;
			v.z = b / 255.0f;
			v.w = a / 255.0f;

			return v;
		}
	};

	//
	struct RGBE
	{
		union {
			struct {
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char e;
			};

			int _value;
		};

		RGBE() { r = 0, g = 0, b = 0, e = 0; }
		RGBE(int c) { _value = c; }
		RGBE(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _e = 0) { r = _r, g = _g, b = _b, e = _e; }
		RGBE(const RGBE & c) { _value = c._value; }

		RGBE & operator =(const RGBE & c)
		{
			r = c.r;
			g = c.g;
			b = c.b;
			e = c.e;

			return *this;
		}

		bool operator ==(const RGBE & c) const
		{
			return _value == c._value;
		}

		bool operator !=(const RGBE & c) const
		{
			return _value != c._value;
		}
	};

	inline RGBE RGBE_FROM(const Float3 & color)
	{
		RGBE c;

		float e;
		e = std::max<float>(color.x, color.y);
		e = std::max<float>(e, color.z);

		if (e <= 1)
		{
			c.r = (unsigned char)(color.x * 255);
			c.g = (unsigned char)(color.y * 255);
			c.b = (unsigned char)(color.z * 255);
			c.e = 0;
		}
		else
		{
			Float3 rk = color / e;
			e = std::min<float>(e, LMAP_RGBE_EXPONENT_MAX) - 1;

			c.r = (unsigned char)(rk.x * 255);
			c.g = (unsigned char)(rk.y * 255);
			c.b = (unsigned char)(rk.z * 255);
			c.e = (unsigned char)(e / (LMAP_RGBE_EXPONENT_MAX - 1) * 255);
		}

		return c;
	}

	inline Float3 RGBE_TO(RGBE color)
	{
		Float3 c;

		float e = 1 + color.e / 255.0f * (LMAP_RGBE_EXPONENT_MAX - 1);
		c.x = color.r / 255.0f * e;
		c.y = color.g / 255.0f * e;
		c.z = color.b / 255.0f * e;

		return c;
	}

}