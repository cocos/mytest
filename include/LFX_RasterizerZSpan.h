#pragma once

#include "LFX_Rasterizer.h"
#include "LFX_Mesh.h"
#include <functional>

namespace LFX {

	struct ZSpan
	{
		int rectx, recty; /* range for clipping */

		int miny1, maxy1, miny2, maxy2;             /* actual filled in range */
		const float *minp1, *maxp1, *minp2, *maxp2; /* vertex pointers detect min/max range in */
		float *span1, *span2;
	};

	void zbuf_alloc_span(ZSpan *zspan, int rectx, int recty);
	void zbuf_free_span(struct ZSpan *zspan);

	void zspan_scanconvert(ZSpan *zpan,
		void *handle,
		float *v1,
		float *v2,
		float *v3,
		void(*func)(void *, int, int, float, float));

	class LFX_ENTRY RasterizerZSpan
	{
		friend void ZSpanCallback(void *handle, int x, int y, float u, float v);

	public:
		struct SpanPixel
		{
			int tid;
			float u, v;
#if 1
			float du_dx, du_dy;
			float dv_dx, dv_dy;
#endif
		};

		struct SpanData
		{
			RasterizerZSpan* rs;
			int tid;
			Vertex A, B, C;
			float du_dx, du_dy;
			float dv_dx, dv_dy;
		};

	public:
		std::function<void(const Float2& texel, const Vertex& v, int mtlId)> E;

		Mesh* _entity;
		int _width, _height;
		int _border;
		int _samples;
		ZSpan _zspan;
		RandomEngine irand;

	public:
		RasterizerZSpan(Mesh* entity, int w, int h, int s, int border);
		~RasterizerZSpan();

		void Rasterize();

	protected:
		void _vout(const Int2& texel, const SpanPixel& p, const SpanData& sd);
	};

}