#pragma once

#include "LFX_Math.h"

namespace LFX {

	class Entity;

	struct LightMapInfo
	{
		int MapIndex;
		Float2 Offset;
		float Scale;
#if LFX_VERSION >= 35
		float Factor;
#else
		float ScaleV;
#endif
	};

	struct Vertex
	{
		Float3 Position;
		Float3 Normal;
		Float3 Tangent;
		Float3 Binormal;
		Float2 UV;
		Float2 LUV;

#ifdef LFX_DEBUG_LUV
		Float3 Color;
#endif // _DEBUG

		Vertex()
		{
			Position = Float3(0, 0, 0);
			Normal = Float3(0, 0, 0);
			Tangent = Float3(0, 0, 0);
			Binormal = Float3(0, 0, 0);
			UV = Float2(0, 0);
			LUV = Float2(0, 0);

#ifdef LFX_DEBUG_LUV
			Color = Float3(0, 0, 0);
#endif
		}

		static void Lerp(Vertex & v, const Vertex & a, const Vertex & b, float k)
		{
			v.Position = a.Position + (b.Position - a.Position) * k;
			v.Normal = a.Normal + (b.Normal - a.Normal) * k;
			v.Tangent = a.Tangent + (b.Tangent - a.Tangent) * k;
			v.Binormal = a.Binormal + (b.Binormal - a.Binormal) * k;
			v.UV = a.UV + (b.UV - a.UV) * k;
			v.LUV = a.LUV + (b.LUV - a.LUV) * k;
#ifdef LFX_DEBUG_LUV
			v.Color = a.Color + (b.Color - a.Color) * k;
#endif

			v.Normal.normalize();
		}

		static void Lerp(Vertex & v, const Vertex & a, const Vertex & b, const Vertex & c, float tu, float tv)
		{
			v.Position = a.Position * (1 - tu - tv) + b.Position * tu + c.Position * tv;
			v.Normal = a.Normal * (1 - tu - tv) + b.Normal * tu + c.Normal * tv;
			v.Tangent = a.Tangent * (1 - tu - tv) + b.Tangent * tu + c.Tangent * tv;
			v.Binormal = a.Binormal * (1 - tu - tv) + b.Binormal * tu + c.Binormal * tv;
			v.UV = a.UV * (1 - tu - tv) + b.UV * tu + c.UV * tv;
			v.LUV = a.LUV * (1 - tu - tv) + b.LUV * tu + c.LUV * tv;
#ifdef LFX_DEBUG_LUV
			v.Color = a.Color * (1 - tu - tv) + b.Color * tu + c.Color * tv;
#endif

			v.Normal.normalize();
			v.Tangent.normalize();
			v.Binormal.normalize();
		}

		Vertex operator +(const Vertex & rk) const
		{
			Vertex v;
			v.Position = Position + rk.Position;
			v.Normal = Normal + rk.Normal;
			v.Tangent = Tangent + rk.Tangent;
			v.Binormal = Binormal + rk.Binormal;
			v.UV = UV + rk.UV;
			v.LUV = LUV + rk.LUV;
#ifdef LFX_DEBUG_LUV
			v.Color = Color + rk.Color;
#endif

			return v;
		}
		
		Vertex operator -(const Vertex & rk) const
		{
			Vertex v;
			v.Position = Position - rk.Position;
			v.Normal = Normal - rk.Normal;
			v.Tangent = Tangent - rk.Tangent;
			v.Binormal = Binormal - rk.Binormal;
			v.UV = UV - rk.UV;
			v.LUV = LUV - rk.LUV;
#ifdef LFX_DEBUG_LUV
			v.Color = Color - rk.Color;
#endif

			return v;
		}

		Vertex operator *(float rk) const
		{
			Vertex v;
			v.Position = Position * rk;
			v.Normal = Normal * rk;
			v.Tangent = Tangent * rk;
			v.Binormal = Binormal * rk;
			v.UV = UV * rk;
			v.LUV = LUV * rk;
#ifdef LFX_DEBUG_LUV
			v.Color = Color * rk;
#endif

			return v;
		}
	};

	struct Triangle
	{
		int Index0, Index1, Index2;
		int MaterialId;

		Triangle()
		{
			Index0 = 0;
			Index1 = 0;
			Index2 = 0;
			MaterialId = 0;
		}
	};

	template<class T>
	struct Rectangle
	{
		T x, y, w, h;

		Rectangle() { x = y = w = h = 0; }
		Rectangle(T _x, T _y, T _w, T _h) : x(_x), y(_y), w(_w), h(_h) {}
		Rectangle(const Rectangle & rk) : x(rk.x), y(rk.y), w(rk.w), h(rk.h) {}
		
		T cx() const
		{
			return (x + w) / 2;
		}

		T cy() const
		{
			return (y + h) / 2;
		}

		T left() const
		{
			return x;
		}

		T top() const
		{
			return y;
		}

		T right() const
		{
			return x + w;
		}

		T bottom() const
		{
			return y + h;
		}

		T square() const
		{
			return w * h;
		}

		bool operator !=(const Rectangle & k) const
		{
			return x != k.x || y != k.y || w != k.w || h != k.h;
		}

		bool operator ==(const Rectangle & k) const
		{
			return x == k.x && y == k.y && w == k.w && h == k.h;
		}

		Rectangle operator |(const Rectangle & rk) const
		{
			T x1 = std::min(x, rk.x);
			T y1 = std::min(y, rk.y);
			T x2 = std::max(right(), rk.right());
			T y2 = std::max(bottom(), rk.bottom());

			return Rectangle(x1, y1, x2 - x1, y2 - y1);
		}

		Rectangle & operator |=(const Rectangle & rk)
		{
			T x1 = std::min(x, rk.x);
			T y1 = std::min(y, rk.y);
			T x2 = std::max(right(), rk.right());
			T y2 = std::max(bottom(), rk.bottom());

			x = x1, y = y1;
			w = x2 - x1, h = y2 - y1;

			return *this;
		}

		Rectangle operator &(const Rectangle & rk) const
		{
			T x1 = std::max(x, rk.x);
			T y1 = std::max(y, rk.y);
			T x2 = std::min(right(), rk.right());
			T y2 = std::min(bottom(), rk.bottom());

			return Rectangle(x1, y1, x2 - x1, y2 - y1);
		}

		Rectangle & operator &=(const Rectangle & rk)
		{
			T x1 = std::max(x, rk.x);
			T y1 = std::max(y, rk.y);
			T x2 = std::min(right(), rk.right());
			T y2 = std::min(bottom(), rk.bottom());

			x = x1, y = y1;
			w = x2 - x1, h = y2 - y1;

			return *this;
		}
	};

	struct Contact
	{
		float td;
		float tu, tv;
		int triIndex;
		Vertex vhit;
		Entity* entity;
		bool facing;
		const void* mtl;
	};

}