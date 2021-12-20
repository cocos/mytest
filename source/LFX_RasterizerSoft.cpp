#include "LFX_RasterizerSoft.h"
#include "LFX_Math.h"
#include "LFX_Mesh.h"
#include "LFX_World.h"

namespace LFX {

	RasterizerSoft::RasterizerSoft(Mesh * mesh, int width, int height)
		: Rasterizer(width, height)
	{
		_mesh = mesh;
		_rchart.resize(width * height);
		for (size_t i = 0; i < _rchart.size(); ++i)
		{
			_rchart[i].MaterialId = -1;
		}
	}

	RasterizerSoft::~RasterizerSoft()
	{
	}

#ifndef LFX_DEBUG_LUV
#define R_OUT_PUT(x, y, v) { _rchart[y * _width + x] = v; }
#else
#define R_OUT_PUT(x, y, v) { _rchart[y * _width + x].Color += Float3(0.5f, 0.5f, 0.5f); }
#endif

	void RasterizerSoft::_DoRasterize(const Vertex & _A, const Vertex & _B, const Vertex & _C, int mtlId, float offset, float scale)
	{
		Vertex ta = _A, tb = _B, tc = _C;

#if 0
		ta.LUV.x = ta.LUV.x * (_width - 0) - 0.0f;
		ta.LUV.y = ta.LUV.y * (_height - 0) - 0.0f;

		tb.LUV.x = tb.LUV.x * (_width - 0) - 0.0f;
		tb.LUV.y = tb.LUV.y * (_height - 0) - 0.0f;

		tc.LUV.x = tc.LUV.x * (_width - 0) - 0.0f;
		tc.LUV.y = tc.LUV.y * (_height - 0) - 0.0f;
#else
		ta.LUV.x = (scale * ta.LUV.x + offset) * (_width - 0);
		ta.LUV.y = (scale * ta.LUV.y + offset)  * (_height - 0);

		tb.LUV.x = (scale * tb.LUV.x + offset) * (_width - 0);
		tb.LUV.y = (scale * tb.LUV.y + offset) * (_height - 0);

		tc.LUV.x = (scale * tc.LUV.x + offset) * (_width - 0);
		tc.LUV.y = (scale * tc.LUV.y + offset) * (_height - 0);
#endif

		//
		const Vertex *PA = &ta, *PB = &tb, *PC = &tc;

		// 确保点A是最上面的点
		if (PA->LUV.y > PB->LUV.y)
			std::swap(PA, PB);
		if (PA->LUV.y > PC->LUV.y)
			std::swap(PA, PC);

		// 确保B在C的左边
		if (PB->LUV.x > PC->LUV.x)
			std::swap(PB, PC);

		Int2 A = Int2((int)PA->LUV.x, (int)PA->LUV.y);
		Int2 B = Int2((int)PB->LUV.x, (int)PB->LUV.y);
		Int2 C = Int2((int)PC->LUV.x, (int)PC->LUV.y);
		Int2 AB = B - A, AC = C - A;

		Vertex v1, v2;
		RVertex v;

		v.MaterialId = mtlId;
		if (B.y > C.y)
		{
			// (B.y < C.y)
			//		A*
			//        
			//			*C
			//	B*
			//

			// 画上半部分
			int cy = A.y, ey = C.y;
			cy = std::max<int>(cy, 0);
			ey = std::min<int>(ey, _height - 1);

			while (cy <= ey)
			{
				float kab = AB.y > 0 ? (float)(cy - A.y) / (float)AB.y : 1;
				float kac = AC.y > 0 ? (float)(cy - A.y) / (float)AC.y : 1;

				int x1 = (int)(A.x + AB.x * kab);
				int x2 = (int)(A.x + AC.x * kac);

				Vertex::Lerp(v1, *PA, *PB, kab);
				Vertex::Lerp(v2, *PA, *PC, kac);

				if (x1 > x2)
				{
					std::swap(x1, x2);
					std::swap(v1, v2);
				}

				int sx = std::max<int>(x1, 0);
				int ex = std::min<int>(x2, _width - 1);

				for (int x = sx; x <= ex; x += 1)
				{
					float k = (x2 - x1) > 0 ? (float)(x - x1) / (float)(x2 - x1) : 1;

					Vertex::Lerp(v, v1, v2, k);

					R_OUT_PUT(x, cy, v);
				}

				cy += 1;
			}

			// 画下半部分
			Int2 CB = Int2(B.x - C.x, B.y - C.y);
			ey = std::min<int>(B.y, _height - 1);

			while (cy <= ey)
			{
				float kab = AB.y > 0 ? (float)(cy - A.y) / (float)AB.y : 1;
				float kcb = CB.y > 0 ? (float)(cy - C.y) / (float)CB.y : 1;

				int x1 = (int)(A.x + AB.x * kab);
				int x2 = (int)(C.x + CB.x * kcb);

				Vertex::Lerp(v1, *PA, *PB, kab);
				Vertex::Lerp(v2, *PC, *PB, kcb);

				if (x1 > x2)
				{
					std::swap(x1, x2);
					std::swap(v1, v2);
				}

				int sx = std::max<int>(x1, 0);
				int ex = std::min<int>(x2, _width - 1);

				for (int x = sx; x <= ex; x += 1)
				{
					float k = (x2 - x1) > 0 ? (float)(x - x1) / (float)(x2 - x1) : 1;

					Vertex::Lerp(v, v1, v2, k);

					R_OUT_PUT(x, cy, v);
				}

				cy += 1;
			}
		}
		else
		{
			// (B.y > C.y)
			//       A*
			//        
			//	 B*    
			//          *C
			//

			// 画上半部分
			int cy = A.y, ey = B.y;
			cy = std::max<int>(cy, 0);
			ey = std::min<int>(ey, _height - 1);

			while (cy <= ey)
			{
				float kab = AB.y > 0 ? (float)(cy - A.y) / (float)AB.y : 1;
				float kac = AC.y > 0 ? (float)(cy - A.y) / (float)AC.y : 1;

				int x1 = (int)(A.x + AB.x * kab);
				int x2 = (int)(A.x + AC.x * kac);

				Vertex::Lerp(v1, *PA, *PB, kab);
				Vertex::Lerp(v2, *PA, *PC, kac);

				if (x1 > x2)
				{
					std::swap(x1, x2);
					std::swap(v1, v2);
				}

				int sx = std::max<int>(x1, 0);
				int ex = std::min<int>(x2, _width - 1);

				for (int x = sx; x <= ex; x += 1)
				{
					float k = (x2 - x1) > 0 ? (float)(x - x1) / (float)(x2 - x1) : 1;

					Vertex::Lerp(v, v1, v2, k);

					R_OUT_PUT(x, cy, v);
				}

				cy += 1;
			}

			// 画下半部分
			Int2 BC = Int2(C.x - B.x, C.y - B.y);
			ey = std::min<int>(C.y, _height - 1);

			if (BC.y > 0)
			{
				while (cy <= ey)
				{
					float kbc = BC.y > 0 ? (float)(cy - B.y) / (float)BC.y : 1;
					float kac = AC.y > 0 ? (float)(cy - A.y) / (float)AC.y : 1;

					int x1 = (int)(B.x + BC.x * kbc);
					int x2 = (int)(A.x + AC.x * kac);

					Vertex::Lerp(v1, *PB, *PC, kbc);
					Vertex::Lerp(v2, *PA, *PC, kac);

					if (x1 > x2)
					{
						std::swap(x1, x2);
						std::swap(v1, v2);
					}

					int sx = std::max<int>(x1, 0);
					int ex = std::min<int>(x2, _width - 1);

					for (int x = sx; x <= ex; x += 1)
					{
						float k = (x2 - x1) > 0 ? (float)(x - x1) / (float)(x2 - x1) : 1;

						Vertex::Lerp(v, v1, v2, k);

						R_OUT_PUT(x, cy, v);
					}

					cy += 1;
				}
			}
		}
	}

	bool PointInTriangle(Float2 P, Float2 A, Float2 B, Float2 C, float & tu, float & tv)
	{
		Float2 v0 = C - A;
		Float2 v1 = B - A;
		Float2 v2 = P - A;

		float dot00 = v0.dot(v0);
		float dot01 = v0.dot(v1);
		float dot02 = v0.dot(v2);
		float dot11 = v1.dot(v1);
		float dot12 = v1.dot(v2);

		float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

		float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
		if (u < 0 || u > 1) // if u out of range, return directly
		{
			return false;
		}

		float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
		if (v < 0 || v > 1) // if v out of range, return directly
		{
			return false;
		}

		tu = u;
		tv = v;

		return u + v <= 1;
	}

	void RasterizerSoft::DoRasterize2()
	{
		for (int i = 0; i < _mesh->NumOfTriangles(); ++i)
		{
			const Triangle & tri = _mesh->_getTriangle(i);
			const Vertex & A = _mesh->_getVertex(tri.Index0);
			const Vertex & B = _mesh->_getVertex(tri.Index1);
			const Vertex & C = _mesh->_getVertex(tri.Index2);

			_DoRasterize(A, B, C, tri.MaterialId, 0, 1);
		}
	}

	bool RS_IsEdge(Rasterizer * rs, int u, int v, int aa)
	{
		int white = 0, black = 0;

		for (int ey = -aa; ey <= aa; ++ey)
		{
			for (int ex = -aa; ex <= aa; ++ex)
			{
				int x = u + ex;
				int y = v + ey;

				if (x >= 0 && x < rs->_width &&
					y >= 0 && y < rs->_height)
				{
					const RVertex & p = rs->_rchart[y * rs->_width + x];
					if (p.MaterialId != -1)
						++white;
					else
						++black;
				}
			}
		}

		return white != 0 && black != 0;
	}

	void RasterizerSoft::DoLighting(const std::vector<Light *> & lights)
	{
#ifdef LFX_FEATURE_EDGA_AA
		const int edgeAA = World::Instance()->GetSetting()->EdgeAA;
#else
		const int edgeAA = 0;
#endif

		int index = 0;
		for (int v = 0; v < _height; ++v)
		{
			for (int u = 0; u < _width; ++u)
			{
				Float3 color = Float3(0, 0, 0);
				const RVertex & bakePoint = _rchart[index];

				if (edgeAA > 0 && RS_IsEdge(this, u, v, edgeAA))
				{
					for (int ey = -edgeAA; ey <= edgeAA; ++ey)
					{
						for (int ex = -edgeAA; ex <= edgeAA; ++ex)
						{
							int x = u + ex;
							int y = v + ey;

							if (x >= 0 && x < _width &&
								y >= 0 && y < _height)
							{
								const RVertex & rvertex = _rchart[y * _width + x];
								if (rvertex.MaterialId != -1)
								{
									for (size_t j = 0; j < lights.size(); ++j)
									{
										color += _mesh->_doLighting(rvertex, rvertex.MaterialId, lights[j]);
									}
								}
							}
						}
					}

					color /= (float)(edgeAA * 2 + 1) * (edgeAA * 2 + 1);
				}
				else
				{
#ifndef LFX_DEBUG_LUV 
					if (bakePoint.MaterialId != -1)
					{
						for (size_t j = 0; j < lights.size(); ++j)
						{
							color += _mesh->_doLighting(bakePoint, bakePoint.MaterialId, lights[j]);
						}
					}
#else
					color = bakePoint.Color;
#endif
				}
				
				_rmap[index++] = color;
			}
		}
	}

}