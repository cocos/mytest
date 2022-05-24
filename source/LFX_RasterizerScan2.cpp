/*
* Slay Engine
* Author: SiZhong.Wang(M-001)
* Copyright: Silvereye Information, Inc. All Rights Reserved.
*/
#include "LFX_RasterizerScan2.h"

namespace LFX {

	RasterizerScan2::RasterizerScan2(Mesh* mesh, int w, int h, int s, int border)
		: _mesh(mesh)
		, _width(w)
		, _height(h)
		, _samples(s)
		, _border(border)
		, Rasterizer(w, h)
	{
	}

	void RasterizerScan2::DoRasterize()
	{
		for (int i = 0; i < _mesh->NumOfTriangles(); ++i)
		{
			const Triangle& tri = _mesh->_getTriangle(i);
			const Vertex& a = _mesh->_getVertex(tri.Index0);
			const Vertex& b = _mesh->_getVertex(tri.Index1);
			const Vertex& c = _mesh->_getVertex(tri.Index2);

			if (a.LUV == b.LUV && a.LUV == c.LUV) {
				continue;
			}

			Vertex A(a), B(b), C(c);
			A.LUV = Rasterizer::Texel(A.LUV, _width, _height, _border);
			B.LUV = Rasterizer::Texel(B.LUV, _width, _height, _border);
			C.LUV = Rasterizer::Texel(C.LUV, _width, _height, _border);

			const float xmin = Min(A.LUV.x, B.LUV.x, C.LUV.x);
			const float ymin = Min(A.LUV.y, B.LUV.y, C.LUV.y);
			const float xmax = Max(A.LUV.x, B.LUV.x, C.LUV.x);
			const float ymax = Max(A.LUV.y, B.LUV.y, C.LUV.y);

			for (float y = ymin; y <= ymax; y += 1.0f) {
				for (float x = xmin; x <= xmax; x += 1.0f) {
					_vout(Float2(x, y), A, B, C, tri.MaterialId);
				}
			}
		}
	}

	bool CalcBarycentricCoord(Float2 P, Float2 A, Float2 B, Float2 C, float& u, float& v)
	{
		Float2 v0 = B - A;
		Float2 v1 = C - A;
		Float2 v2 = P - A;

		float d00 = v0.dot(v0);
		float d01 = v0.dot(v1);
		float d02 = v0.dot(v2);
		float d11 = v1.dot(v1);
		float d12 = v1.dot(v2);
		float denom = (d00 * d11 - d01 * d01);

		u = (d11 * d02 - d01 * d12) / denom;
		v = (d00 * d12 - d01 * d02) / denom;

		// u out of range
		if (u < 0 || u > 1) {
			return false;
		}

		// v out of range
		if (v < 0 || v > 1) {
			return false;
		}

		return u + v <= 1;
	}

	void RasterizerScan2::_vout(const Float2& texel, const Vertex& A, const Vertex& B, const Vertex& C, int mtlId)
	{
		if (_samples > 1) {
			const int sq = _samples;
			for (int j = 0; j < sq; ++j) {
				for (int i = 0; i < sq; ++i) {
					Float2 ftexel;
					ftexel.x = texel.x + i / (float)(sq - 1) - 0.5f;
					ftexel.y = texel.y + j / (float)(sq - 1) - 0.5f;
					_vfunc(texel, A, B, C, mtlId);
				}
			}
		}
		else {
			_vfunc(texel, A, B, C, mtlId);
		}
	}

	void RasterizerScan2::_vfunc(const Float2& texel, const Vertex& A, const Vertex& B, const Vertex& C, int mtlId)
	{
		if (TexelIsOut(texel, _width, _height)) {
			return;
		}

		float tu, tv;
		if (!CalcBarycentricCoord(texel, A.LUV, B.LUV, C.LUV, tu, tv)) {
			return;
		}

		Vertex v = A * (1 - tu - tv) + B * tu + C * tv;
		v.Normal.normalize();
		v.Tangent.normalize();
		v.Binormal.normalize();
		F(texel, v, mtlId);
	}

}