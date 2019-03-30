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
		static void DoBlur(Float3 * data, int xMapSize, int zMapSize, int stride);

	public:
		int _width, _height;
		std::vector<Float3> _rmap;
		std::vector<RVertex> _rchart;

	public:
		Rasterizer(int width, int height);
		~Rasterizer();

		virtual void DoRasterize2() = 0;

		virtual void DoLighting(const std::vector<Light *> & lights) = 0;
		virtual void DoAmbientOcclusion() = 0;
	};


}