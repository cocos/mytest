#include "LFX_RasterizerZSpan.h"
#include "LFX_World.h"

namespace LFX {

	RasterizerZSpan::RasterizerZSpan(Mesh* entity, int w, int h, int s, int border)
		: _entity(entity)
		, _width(w)
		, _height(h)
		, _samples(s)
		, _border(border)
	{
#if 0
		_rchart.resize(w * h);
		for (auto& p : _rchart) {
			p.tid = -1;
		}
#endif

		zbuf_alloc_span(&_zspan, w, h);
	}

	RasterizerZSpan::~RasterizerZSpan()
	{
		zbuf_free_span(&_zspan);
	}

	void zspan_differentials(RasterizerZSpan::SpanData* sd, const float *uv1, const float *uv2, const float *uv3)
	{
		float A;

		/* assumes dPdu = P1 - P3 and dPdv = P2 - P3 */
		A = (uv2[0] - uv1[0]) * (uv3[1] - uv1[1]) - (uv3[0] - uv1[0]) * (uv2[1] - uv1[1]);

		if (fabsf(A) > FLT_EPSILON) {
			A = 0.5f / A;

			sd->du_dx = (uv2[1] - uv3[1]) * A;
			sd->dv_dx = (uv3[1] - uv1[1]) * A;

			sd->du_dy = (uv3[0] - uv2[0]) * A;
			sd->dv_dy = (uv1[0] - uv3[0]) * A;
		}
		else {
			sd->du_dx = sd->du_dy = 0.0f;
			sd->dv_dx = sd->dv_dy = 0.0f;
		}
	}

	void ZSpanCallback(void *handle, int x, int y, float u, float v)
	{
		RasterizerZSpan::SpanData *bd = (RasterizerZSpan::SpanData*)handle;
#if 0
		const int i = y * bd->rs->_width + x;
		RasterizerZSpan::SpanPixel& pixel = bd->rs->_rchart[i];
#endif

		RasterizerZSpan::SpanPixel pixel;
		pixel.tid = bd->tid;
		pixel.u = u;
		pixel.v = v;
		pixel.du_dx = bd->du_dx;
		pixel.du_dy = bd->du_dy;
		pixel.dv_dx = bd->dv_dx;
		pixel.dv_dy = bd->dv_dy;

		bd->rs->_vout(Int2(x, y), pixel, *bd);
	}

	void RasterizerZSpan::Rasterize()
	{
		SpanData bd;
		bd.rs = this;
		for (int t = 0; t < _entity->NumOfTriangles(); ++t)
		{
			const Triangle& tri = _entity->_getTriangle(t);

			bd.tid = t;
			bd.A = _entity->_getVertex(tri.Index0);
			bd.B = _entity->_getVertex(tri.Index1);
			bd.C = _entity->_getVertex(tri.Index2);

			bd.A.LUV = Rasterizer::Texel(bd.A.LUV, _width, _height, _border);
			bd.B.LUV = Rasterizer::Texel(bd.B.LUV, _width, _height, _border);
			bd.C.LUV = Rasterizer::Texel(bd.C.LUV, _width, _height, _border);

			zspan_differentials(&bd, &bd.A.LUV[0], &bd.B.LUV[0], &bd.C.LUV[0]);
			zspan_scanconvert(&_zspan, &bd, &bd.A.LUV[0], &bd.B.LUV[0], &bd.C.LUV[0], ZSpanCallback);
		}

#if 0
		for (int j = 0; j < _height; ++j) {
			for (int i = 0; i < _width; ++i) {
					RSPixel* pixel = &_rchart[j * _width + i];
					if (pixel->tid == -1) {
						continue;
					}

				for (int sample = 0; sample < _samples; ++sample) {
					Float2 texel((float)i, (float)j);
					texel.x += (UniformDistribution(RandEngine) - 0.5f);
					texel.y += (UniformDistribution(RandEngine) - 0.5f);

					const Triangle& tri = _entity->GetTriangles().at(pixel->tid);
					Vertex A = _entity->GetVertices().at(tri.Index0);
					Vertex B = _entity->GetVertices().at(tri.Index1);
					Vertex C = _entity->GetVertices().at(tri.Index2);

					A.LUV = Rasterizer::Texel(A.LUV, _width, _height, 1, 0);
					B.LUV = Rasterizer::Texel(B.LUV, _width, _height, 1, 0);
					C.LUV = Rasterizer::Texel(C.LUV, _width, _height, 1, 0);

					float tu, tv;
					if (!Rasterizer::PointInTriangle(texel, A.LUV, B.LUV, C.LUV, tu, tv)) {
						continue;
					}

					Vertex v = A * (1 - tu - tv) + B * tu + C * tv;
					v.Normal.Normalize();
					v.Tangent.Normalize();
					v.Binormal.Normalize();
					F(this, texel, RVertex(v, tri.MaterialId));
				}
			}
		}
#endif
	}

	void RasterizerZSpan::_vout(const Int2& texel, const SpanPixel& p, const SpanData& sd)
	{
		const Vertex& A = sd.A;
		const Vertex& B = sd.B;
		const Vertex& C = sd.C;
		const int mtlId = _entity->_getTriangle(p.tid).MaterialId;

		Float2 ftexel;
		if (_samples > 1) {
			for (int s = 0; s < _samples; ++s) {
				ftexel.x = texel.x + (irand.UniformDistribution() - 0.5f);
				ftexel.y = texel.y + (irand.UniformDistribution() - 0.5f);

				float tu, tv;
				if (!Rasterizer::PointInTriangle(ftexel, A.LUV, B.LUV, C.LUV, tu, tv)) {
					continue;
				}

				Vertex v = A * (1 - tu - tv) + B * tu + C * tv;
				v.Normal.normalize();
				v.Tangent.normalize();
				v.Binormal.normalize();
				F(ftexel, v, mtlId);
			}
		}
		else {
			ftexel.x = (float)texel.x;
			ftexel.y = (float)texel.y;

			float tu, tv;
			if (!Rasterizer::PointInTriangle(ftexel, A.LUV, B.LUV, C.LUV, tu, tv)) {
				return;
			}

			Vertex v = A * (1 - tu - tv) + B * tu + C * tv;
			v.Normal.normalize();
			v.Tangent.normalize();
			v.Binormal.normalize();
			F(ftexel, v, mtlId);
		}
	}

#pragma warning(push)
#pragma warning(disable: 4244)
	/* ****************** Spans ******************************* */

	/* each zbuffer has coordinates transformed to local rect coordinates, so we can simply clip */
	void zbuf_alloc_span(ZSpan *zspan, int rectx, int recty)
	{
		memset(zspan, 0, sizeof(ZSpan));

		zspan->rectx = rectx;
		zspan->recty = recty;

		zspan->span1 = (float*)malloc(recty * sizeof(float));
		zspan->span2 = (float*)malloc(recty * sizeof(float));
	}

	void zbuf_free_span(ZSpan *zspan)
	{
		if (zspan) {
			if (zspan->span1) {
				free(zspan->span1);
			}
			if (zspan->span2) {
				free(zspan->span2);
			}
			zspan->span1 = zspan->span2 = NULL;
		}
	}

	/* reset range for clipping */
	static void zbuf_init_span(ZSpan *zspan)
	{
		zspan->miny1 = zspan->miny2 = zspan->recty + 1;
		zspan->maxy1 = zspan->maxy2 = -1;
		zspan->minp1 = zspan->maxp1 = zspan->minp2 = zspan->maxp2 = NULL;
	}

	static void zbuf_add_to_span(ZSpan *zspan, const float v1[2], const float v2[2])
	{
		const float *minv, *maxv;
		float *span;
		float xx1, dx0, xs0;
		int y, my0, my2;

		if (v1[1] < v2[1]) {
			minv = v1;
			maxv = v2;
		}
		else {
			minv = v2;
			maxv = v1;
		}

		my0 = ceil(minv[1]);
		my2 = floor(maxv[1]);

		if (my2 < 0 || my0 >= zspan->recty) {
			return;
		}

		/* clip top */
		if (my2 >= zspan->recty) {
			my2 = zspan->recty - 1;
		}
		/* clip bottom */
		if (my0 < 0) {
			my0 = 0;
		}

		if (my0 > my2) {
			return;
		}
		/* if (my0>my2) should still fill in, that way we get spans that skip nicely */

		xx1 = maxv[1] - minv[1];
		if (xx1 > FLT_EPSILON) {
			dx0 = (minv[0] - maxv[0]) / xx1;
			xs0 = dx0 * (minv[1] - my2) + minv[0];
		}
		else {
			dx0 = 0.0f;
			xs0 = std::min(minv[0], maxv[0]);
		}

		/* empty span */
		if (zspan->maxp1 == NULL) {
			span = zspan->span1;
		}
		else { /* does it complete left span? */
			if (maxv == zspan->minp1 || minv == zspan->maxp1) {
				span = zspan->span1;
			}
			else {
				span = zspan->span2;
			}
		}

		if (span == zspan->span1) {
			//      printf("left span my0 %d my2 %d\n", my0, my2);
			if (zspan->minp1 == NULL || zspan->minp1[1] > minv[1]) {
				zspan->minp1 = minv;
			}
			if (zspan->maxp1 == NULL || zspan->maxp1[1] < maxv[1]) {
				zspan->maxp1 = maxv;
			}
			if (my0 < zspan->miny1) {
				zspan->miny1 = my0;
			}
			if (my2 > zspan->maxy1) {
				zspan->maxy1 = my2;
			}
		}
		else {
			//      printf("right span my0 %d my2 %d\n", my0, my2);
			if (zspan->minp2 == NULL || zspan->minp2[1] > minv[1]) {
				zspan->minp2 = minv;
			}
			if (zspan->maxp2 == NULL || zspan->maxp2[1] < maxv[1]) {
				zspan->maxp2 = maxv;
			}
			if (my0 < zspan->miny2) {
				zspan->miny2 = my0;
			}
			if (my2 > zspan->maxy2) {
				zspan->maxy2 = my2;
			}
		}

		for (y = my2; y >= my0; y--, xs0 += dx0) {
			/* xs0 is the xcoord! */
			span[y] = xs0;
		}
	}

	/*-----------------------------------------------------------*/
	/* Functions                                                 */
	/*-----------------------------------------------------------*/

	/* Scanconvert for strand triangles, calls func for each x, y coordinate
	 * and gives UV barycentrics and z. */

	void zspan_scanconvert(ZSpan *zspan,
		void *handle,
		float *v1,
		float *v2,
		float *v3,
		void(*func)(void *, int, int, float, float))
	{
		float x0, y0, x1, y1, x2, y2, z0, z1, z2;
		float u, v, uxd, uyd, vxd, vyd, uy0, vy0, xx1;
		const float *span1, *span2;
		int i, j, x, y, sn1, sn2, rectx = zspan->rectx, my0, my2;

		/* init */
		zbuf_init_span(zspan);

		/* set spans */
		zbuf_add_to_span(zspan, v1, v2);
		zbuf_add_to_span(zspan, v2, v3);
		zbuf_add_to_span(zspan, v3, v1);

		/* clipped */
		if (zspan->minp2 == NULL || zspan->maxp2 == NULL) {
			return;
		}

		my0 = std::max(zspan->miny1, zspan->miny2);
		my2 = std::min(zspan->maxy1, zspan->maxy2);

		//  printf("my %d %d\n", my0, my2);
		if (my2 < my0) {
			return;
		}

		/* ZBUF DX DY, in floats still */
		x1 = v1[0] - v2[0];
		x2 = v2[0] - v3[0];
		y1 = v1[1] - v2[1];
		y2 = v2[1] - v3[1];

		z1 = 1.0f; /* (u1 - u2) */
		z2 = 0.0f; /* (u2 - u3) */

		x0 = y1 * z2 - z1 * y2;
		y0 = z1 * x2 - x1 * z2;
		z0 = x1 * y2 - y1 * x2;

		if (z0 == 0.0f) {
			return;
		}

		xx1 = (x0 * v1[0] + y0 * v1[1]) / z0 + 1.0f;
		uxd = -(double)x0 / (double)z0;
		uyd = -(double)y0 / (double)z0;
		uy0 = ((double)my2) * uyd + (double)xx1;

		z1 = -1.0f; /* (v1 - v2) */
		z2 = 1.0f;  /* (v2 - v3) */

		x0 = y1 * z2 - z1 * y2;
		y0 = z1 * x2 - x1 * z2;

		xx1 = (x0 * v1[0] + y0 * v1[1]) / z0;
		vxd = -(double)x0 / (double)z0;
		vyd = -(double)y0 / (double)z0;
		vy0 = ((double)my2) * vyd + (double)xx1;

		/* correct span */
		span1 = zspan->span1 + my2;
		span2 = zspan->span2 + my2;

		for (i = 0, y = my2; y >= my0; i++, y--, span1--, span2--) {

			sn1 = floor(std::min(*span1, *span2));
			sn2 = floor(std::max(*span1, *span2));
			sn1++;

			if (sn2 >= rectx) {
				sn2 = rectx - 1;
			}
			if (sn1 < 0) {
				sn1 = 0;
			}

			u = (((double)sn1 * uxd) + uy0) - (i * uyd);
			v = (((double)sn1 * vxd) + vy0) - (i * vyd);

			for (j = 0, x = sn1; x <= sn2; j++, x++) {
				func(handle, x, y, u + (j * uxd), v + (j * vxd));
			}
		}
	}

#pragma warning(pop)
}