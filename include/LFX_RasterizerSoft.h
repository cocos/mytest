#pragma once

#include "LFX_Rasterizer.h"
#include "LFX_Mesh.h"

namespace LFX {

	class RasterizerSoft : public Rasterizer
	{
	public:
		Mesh * _mesh;

	public:
		RasterizerSoft(Mesh * mesh, int width, int height);
		~RasterizerSoft();

		void _DoRasterize(const Vertex & A, const Vertex & B, const Vertex & C, int mtlId, float offset, float scale);
		virtual void DoRasterize2();

		virtual void DoLighting(const std::vector<Light *> & lights);
	};

	extern bool RS_IsEdge(Rasterizer * rs, int u, int v, int aa);

}