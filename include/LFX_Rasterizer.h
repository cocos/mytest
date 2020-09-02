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

		static void Blur(Float3* data, int w, int h, int stride);
		static void Optimize(Float4* lmap, int w, int h, int border);
		static bool PointInTriangle(Float2 P, Float2 A, Float2 B, Float2 C, float& tu, float& tv);

	public:
		int _width, _height;
		std::vector<Float3> _rmap;
		std::vector<RVertex> _rchart;

	public:
		Rasterizer(int width, int height);
		~Rasterizer();

		virtual void DoRasterize2() = 0;

		virtual void DoLighting(const std::vector<Light *> & lights) = 0;
	};


}