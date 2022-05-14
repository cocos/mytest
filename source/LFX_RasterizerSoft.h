#pragma once

#include "LFX_Rasterizer.h"
#include "LFX_Mesh.h"

namespace LFX {

	class RasterizerSoft : public Rasterizer
	{
	public:
		Mesh* _mesh;
		int _width, _height;
		std::vector<RVertex> _rchart;

	public:
		RasterizerSoft(Mesh * mesh, int width, int height);
		~RasterizerSoft();

		void _rasterize(const Vertex & A, const Vertex & B, const Vertex & C, int mtlId, float offset, float scale);
		virtual void DoRasterize();
	};

	extern bool RS_IsEdge(RasterizerSoft* rs, int u, int v, int aa);

}