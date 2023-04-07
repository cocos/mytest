#pragma once

#include "LFX_Types.h"
#include "LFX_Light.h"

namespace LFX {

	struct RVertex : public Vertex
	{
		int MaterialId;

		RVertex()
		{
			MaterialId = 0;
		}
	};

	class LFX_ENTRY Rasterizer
	{
	public:
		static Float2 Texel(const Float2& uv, int w, int h, int border);
		static Float2 Texel(const Float2& uv, int w, int h, int border, const Float2& tm);

		static void Blur(float* data, int w, int h, int stride, int texels);
		static void Blur(Float3* data, int w, int h, int stride, int texels);
		static void Blur(Float4* data, int w, int h, int stride, int texels);
		static void Blur(LightmapValue* data, int w, int h, int stride, int texels);
		static void Optimize(Float4* lmap, int w, int h, int border);
		static bool TexelIsOut(const Float2& texel, int w, int h);
		static bool PointInTriangle(Float2 P, Float2 A, Float2 B, Float2 C, float& tu, float& tv);

	public:
		std::vector<Float3> _rmap;

	public:
		Rasterizer(int width, int height);
		virtual ~Rasterizer() {}

		virtual void DoRasterize() = 0;
	};

}