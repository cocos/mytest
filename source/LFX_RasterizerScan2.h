/*
* Slay Engine
* Author: SiZhong.Wang(M-001)
* Copyright: Silvereye Information, Inc. All Rights Reserved.
*/
#pragma once

#include "LFX_Rasterizer.h"
#include "LFX_Mesh.h"
#include <functional>

namespace LFX {

	class LFX_ENTRY RasterizerScan2 : public Rasterizer
	{
	public:
		Mesh* _mesh;
		int _width, _height;
		int _border;
		int _samples;
		RandomEngine irand;
		std::function<void(const Float2& texel, const Vertex& v, int mtlId)> F;

	public:
		RasterizerScan2(Mesh* mesh, int w, int h, int s, int border);

		void DoRasterize() override;

	protected:
		void _vout(const Float2& texel, const Vertex& A, const Vertex& B, const Vertex& C, int mtlId);
		void _vfunc(const Float2& texel, const Vertex& A, const Vertex& B, const Vertex& C, int mtlId);
	};

}