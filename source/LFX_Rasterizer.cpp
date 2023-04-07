#include "LFX_Rasterizer.h"

namespace LFX {

	Rasterizer::Rasterizer(int width, int height)
	{
		assert(width > 16 && height > 16);
		_rmap.resize(width * height);
	}

	Float2 Rasterizer::Texel(const Float2& uv, int w, int h, int border)
	{
		return Texel(uv, w, h, border, Float2(1, 0.5f));
	}

	Float2 Rasterizer::Texel(const Float2& uv, int w, int h, int border, const Float2& tm)
	{
		Float2 t;
#if 0
		t.x = Math::Floor(uv.x * (w - tm.x - border * 2)) + tm.y + border;
		t.y = Math::Floor(uv.y * (h - tm.x - border * 2)) + tm.y + border;
#else
		t.x = uv.x * (w - tm.x - border * 2) + tm.y + border;
		t.y = uv.y * (h - tm.x - border * 2) + tm.y + border;
#endif
		return t;
	}

	void Rasterizer::Blur(float* data, int w, int h, int stride, int texels)
	{
		float* temp = new float[w * h];

		int index = 0;
		for (int v = 0; v < h; ++v)
		{
			for (int u = 0; u < w; ++u)
			{
				float color;

				for (int y = -texels; y <= texels; ++y)
				{
					for (int x = -texels; x <= texels; ++x)
					{
						int s = Clamp<int>(u + x, 0, w - 1);
						int t = Clamp<int>(v + y, 0, h - 1);

						color += data[(t * stride) + s];
					}
				}

				color /= 9.0f;

				temp[index++] = color;
			}
		}

		for (int i = 0; i < h; ++i)
		{
			memcpy(data + i * stride, temp + i * w, w * sizeof(float));
		}

		delete[] temp;
	}

	void Rasterizer::Blur(Float3* data, int w, int h, int stride, int texels)
	{
		Float3* temp = new Float3[w * h];

		int index = 0;
		for (int v = 0; v < h; ++v)
		{
			for (int u = 0; u < w; ++u)
			{
				Float3 color(0, 0, 0);

				for (int y = -texels; y <= texels; ++y)
				{
					for (int x = -texels; x <= texels; ++x)
					{
						int s = Clamp<int>(u + x, 0, w - 1);
						int t = Clamp<int>(v + y, 0, h - 1);

						color += data[(t * stride) + s];
					}
				}

				color /= 9.0f;

				temp[index++] = color;
			}
		}

		for (int i = 0; i < h; ++i)
		{
			memcpy(data + i * stride, temp + i * w, w * sizeof(Float3));
		}

		delete[] temp;
	}

	void Rasterizer::Blur(Float4* data, int w, int h, int stride, int texels)
	{
		Float4* temp = new Float4[w * h];

		int index = 0;
		for (int v = 0; v < h; ++v)
		{
			for (int u = 0; u < w; ++u)
			{
				Float4 color(0, 0, 0, 0);

				for (int y = -texels; y <= texels; ++y)
				{
					for (int x = -texels; x <= texels; ++x)
					{
						int s = Clamp<int>(u + x, 0, w - 1);
						int t = Clamp<int>(v + y, 0, h - 1);

						color += data[(t * stride) + s];
					}
				}

				color /= 9.0f;

				temp[index++] = color;
			}
		}

		for (int i = 0; i < h; ++i)
		{
			memcpy(data + i * stride, temp + i * w, w * sizeof(Float4));
		}

		delete[] temp;
	}

	void Rasterizer::Blur(LightmapValue* data, int w, int h, int stride, int texels)
	{
		LightmapValue* temp = new LightmapValue[w * h];

		int index = 0;
		for (int v = 0; v < h; ++v)
		{
			for (int u = 0; u < w; ++u)
			{
				LightmapValue color;
				color.Diffuse.zero();
				color.Shadow = 0;
				color.AO = 0;

				for (int y = -texels; y <= texels; ++y)
				{
					for (int x = -texels; x <= texels; ++x)
					{
						int s = Clamp<int>(u + x, 0, w - 1);
						int t = Clamp<int>(v + y, 0, h - 1);

						color.Diffuse += data[(t * stride) + s].Diffuse;
						color.Shadow += data[(t * stride) + s].Shadow;
						color.AO += data[(t * stride) + s].AO;
					}
				}

				color.Diffuse /= 9.0f;
				color.Shadow /= 9.0f;
				color.AO /= 9.0f;

				temp[index++] = color;
			}
		}

		for (int i = 0; i < h; ++i)
		{
			memcpy(data + i * stride, temp + i * w, w * sizeof(LightmapValue));
		}

		delete[] temp;
	}

	void Rasterizer::Optimize(Float4* lmap, int w, int h, int border)
	{
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				float s = lmap[y * w + x].w;
				if (s > 0) {
					continue;
				}

				Float3 color;
				float samples = 0;
				for (int j = -border; j <= border; ++j) {
					for (int i = -border; i <= border; ++i) {
						int cx = x + i;
						int cy = y + j;

						if (cx < 0 || cx >= w || cy < 0 || cy >= h) {
							continue;
						}

						Float4 lc = lmap[cy * w + cx];
						if (lc.w > 0) {
							const int d = 1;//std::max(1, std::max(abs(j), abs(i)));
							color += Float3(lc.x, lc.y, lc.z) / (float)d;
							samples += 1.0f / (float)d;
						}
					}
				}

				if (samples > 0) {
					color *= 1.0f / samples;
					lmap[y * w + x] = Float4(color.x, color.y, color.z, 0);
				}
			}
		}
	}

	bool Rasterizer::TexelIsOut(const Float2& texel, int w, int h)
	{
		if (texel.x < 0 || texel.x > w - 1) {
			return true;
		}
		if (texel.y < 0 || texel.y > h - 1) {
			return true;
		}

		return false;
	}

	bool Rasterizer::PointInTriangle(Float2 P, Float2 A, Float2 B, Float2 C, float& u, float& v)
	{
		Float2 v0 = B - A;
		Float2 v1 = C - A;
		Float2 v2 = P - A;

		float dot00 = v0.dot(v0);
		float dot01 = v0.dot(v1);
		float dot02 = v0.dot(v2);
		float dot11 = v1.dot(v1);
		float dot12 = v1.dot(v2);

		float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

		u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
		v = (dot00 * dot12 - dot01 * dot02) * inverDeno;

		if (u < 0 || u > 1) // if u out of range, return directly
		{
			return false;
		}

		if (v < 0 || v > 1) // if v out of range, return directly
		{
			return false;
		}

		return u + v <= 1;
	}

}