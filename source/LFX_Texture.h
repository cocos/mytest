#pragma once

#include "LFX_Math.h"

namespace LFX {

	struct Texture
	{
		String name;
		int width;
		int height;
		int channels;
		unsigned char * data;

		Texture()
		{
			channels = 0;
			width = 0;
			height = 0;
			data = NULL;
		}

		~Texture()
		{
		}

		int GetWidth() const
		{
			return width;
		}

		int GetHeight() const
		{
			return height;
		}

		bool GetColor(Float4 & c, int x, int y) const
		{
			assert(x < width && y < height);

			unsigned char r, g, b, a;

			if (channels == 4)
			{
				b = data[(y * width + x) * 4 + 0];
				g = data[(y * width + x) * 4 + 1];
				r = data[(y * width + x) * 4 + 2];
				a = data[(y * width + x) * 4 + 3];

				c.x = r / 255.0f;
				c.y = g / 255.0f;
				c.z = b / 255.0f;
				c.w = a / 255.0f;

				return true;
			}
			else if (channels == 3)
			{
				r = data[(y * width + x) * 3 + 0];
				g = data[(y * width + x) * 3 + 1];
				b = data[(y * width + x) * 3 + 2];
				a = 255;

				c.x = r / 255.0f;
				c.y = g / 255.0f;
				c.z = b / 255.0f;
				c.w = a / 255.0f;

				return true;
			}

			return false;
		}

		Float4 SampleColor(float u, float v)
		{
#if 0
			while (u < 0)
				u += 1;
			while (u > 1)
				u -= 1;
			while (v < 0)
				v += 1;
			while (v > 1)
				v -= 1;
#else
			u = Clamp(u, 0.0f, 1.0f);
			v = Clamp(v, 0.0f, 1.0f);
#endif

			float fu = u * (width - 1);
			float fv = v * (height- 1);

			int iu0 = (int)fu;
			int iv0 = (int)fv;
			int iu1 = iu0 + 1;
			int iv1 = iv0 + 1;

			if (iu1 > width - 1)
				iu1 = iu0;

			if (iv1 > height - 1)
				iv1 = iv0;

			Float4 c0, c1, c2, c3;

			if (!GetColor(c0, iu0, iv0))
				return Float4(1, 1, 1, 1);
			if (!GetColor(c1, iu1, iv0))
				return Float4(1, 1, 1, 1);
			if (!GetColor(c2, iu0, iv1))
				return Float4(1, 1, 1, 1);
			if (!GetColor(c3, iu1, iv1))
				return Float4(1, 1, 1, 1);

			float du = fu - iu0;
			float dv = fv - iv0;

			Float4 c4 = c0 + (c1 - c0) * du;
			Float4 c5 = c2 + (c3 - c2) * du;

			return c4 + (c5 - c4) * dv;
		}
	};

}