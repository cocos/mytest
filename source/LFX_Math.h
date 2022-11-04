#pragma once

#include "LFX_Types.h"
#include "LFX_Vec2.h"
#include "LFX_Vec3.h"
#include "LFX_Vec4.h"
#include "LFX_Mat3.h"
#include "LFX_Color.h"
#include "LFX_Aabb.h"

namespace LFX {

#define DEG_TO_RAD_FACTORY (Pi / 180.0f)
#define RAD_TO_DEG_FACTORY (180.0f / Pi)

	// Constants
	const float Pi = 3.141592654f;
	const float Pi2 = 6.283185307f;
	const float Pi_2 = 1.570796327f;
	const float Pi_4 = 0.7853981635f;
	const float InvPi = 0.318309886f;
	const float InvPi2 = 0.159154943f;
	const float FP16Max = 65000.0f;
	const float FP16Scale = 0.0009765625f;

	//
	typedef Vec2T<int> Int2;
	typedef Vec3T<int> Int3;
	typedef Vec4T<int> Int4;

	typedef Vec2T<float> Float2;
	typedef Vec3T<float> Float3;
	typedef Vec4T<float> Float4;

	struct Ray
	{
		Float3 orig;
		Float3 dir;
	};

	template<typename T>
	void Swap(T& a, T& b)
	{
		T tmp = a;
		a = b;
		b = tmp;
	}

	template<typename T>
	T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	T Max(T a, T b)
	{
		return a < b ? b : a;
	}

	template <class T>
	inline T Min(const T a, const T b, const T c)
	{
		return a < b ? (a < c ? a : c) : (b < c ? b : c);
	}

	template <class T>
	inline T Max(const T a, const T b, const T c)
	{
		return a > b ? (a > c ? a : c) : (b > c ? b : c);
	}

	inline float Round(float r)
	{
		return (r > 0.0f) ? std::floor(r + 0.5f) : std::ceil(r - 0.5f);
	}

	inline float RandFloat()
	{
		return rand() / static_cast<float>(RAND_MAX);
	}

	template<typename T>
	T Square(T x)
	{
		return x * x;
	}

	template<typename T>
	T Clamp(T val, T min, T max)
	{
		assert(max >= min);

		if (val < min)
			val = min;
		else if (val > max)
			val = max;
		return val;
	}

	template <class T>
	inline T Lerp(const T & a, const T & b, float t)
	{
		return a + (b - a) * t;
	}

	inline float DegreeToRadian(float degree)
	{
		return degree * DEG_TO_RAD_FACTORY;
	}

	inline float RadianToDegree(float rad)
	{
		return rad * RAD_TO_DEG_FACTORY;
	}

	inline float Distance(const Float3 & a, const Float3 & b)
	{
		return (a - b).len();
	}

	inline float DistanceSq(const Float3 & a, const Float3 & b)
	{
		return (a - b).lenSqr();
	}

	inline float Pow(float x, float y)
	{
		return std::pow(x, y);
	}

	inline Float2 Pow(const Float2 & x, float y)
	{
		Float2 r;
		r.x = std::pow(x.x, y);
		r.y = std::pow(x.y, y);

		return r;
	}

	inline Float3 Pow(const Float3 & x, float y)
	{
		Float3 r;
		r.x = std::pow(x.x, y);
		r.y = std::pow(x.y, y);
		r.z = std::pow(x.z, y);

		return r;
	}

	inline int NearestPow2(int x)
	{
		if (x > 0)
		{
			--x;
			x |= x >> 1;
			x |= x >> 2;
			x |= x >> 4;
			x |= x >> 8;
			x |= x >> 16;
			return ++x;
		}

		return 0;
	}

	inline float Saturate(float r)
	{
		return Clamp<float>(r, 0, 1);
	}

	inline void Saturate(Float3 & r)
	{
		r.x = Clamp<float>(r.x, 0, 1);
		r.y = Clamp<float>(r.y, 0, 1);
		r.z = Clamp<float>(r.z, 0, 1);
	}

	inline void Saturate(Float4 & r)
	{
		r.x = Clamp<float>(r.x, 0, 1);
		r.y = Clamp<float>(r.y, 0, 1);
		r.z = Clamp<float>(r.z, 0, 1);
		r.w = Clamp<float>(r.w, 0, 1);
	}

	inline Float2 Minimum(const Float2 & a, const Float2 & b)
	{
		Float2 r;
		r.x = a.x < b.x ? a.x : b.x;
		r.y = a.y < b.y ? a.y : b.y;

		return r;
	}

	inline Float3 Minimum(const Float3 & a, const Float3 & b)
	{
		Float3 r;
		r.x = a.x < b.x ? a.x : b.x;
		r.y = a.y < b.y ? a.y : b.y;
		r.z = a.z < b.z ? a.z : b.z;

		return r;
	}

	inline Float2 Maximum(const Float2 & a, const Float2 & b)
	{
		Float2 r;
		r.x = a.x > b.x ? a.x : b.x;
		r.y = a.y > b.y ? a.y : b.y;

		return r;
	}

	inline Float3 Maximum(const Float3 & a, const Float3 & b)
	{
		Float3 r;
		r.x = a.x > b.x ? a.x : b.x;
		r.y = a.y > b.y ? a.y : b.y;
		r.z = a.z > b.z ? a.z : b.z;

		return r;
	}

	inline Float3 Reflect(const Float3 & v, const Float3 & n)
	{
		return v - 2.0f * n * Float3::Dot(v, n);
	}

	inline bool Intersect(const Ray & ray, float * _dist, const Aabb & rk)
	{
		float dist;
		bool hit = false;

		if (ray.orig > rk.minimum && ray.orig < rk.maximum)
		{
			//inside
			dist = 0;
			hit = true;
		}
		else
		{
			//outside
			float t;
			float lowt = 0.0f;
			Float3 hitpos;

			//check each face

			//minimum x
			if (ray.orig.x <= rk.minimum.x && ray.dir.x > 0.0f)
			{
				t = (rk.minimum.x - ray.orig.x) / ray.dir.x;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.y >= rk.minimum.y && hitpos.y <= rk.maximum.y &&
						hitpos.z >= rk.minimum.z && hitpos.z <= rk.maximum.z &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			//maximum x
			if (ray.orig.x >= rk.maximum.x && ray.dir.x < 0.0f)
			{
				t = (rk.maximum.x - ray.orig.x) / ray.dir.x;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.y >= rk.minimum.y && hitpos.y <= rk.maximum.y &&
						hitpos.z >= rk.minimum.z && hitpos.z <= rk.maximum.z &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			//minimum y
			if (ray.orig.y <= rk.minimum.y && ray.dir.y > 0.0f)
			{
				t = (rk.minimum.y - ray.orig.y) / ray.dir.y;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.x >= rk.minimum.x && hitpos.x <= rk.maximum.x &&
						hitpos.z >= rk.minimum.z && hitpos.z <= rk.maximum.z &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			//maximum y
			if (ray.orig.y >= rk.maximum.y && ray.dir.y < 0.0f)
			{
				t = (rk.maximum.y - ray.orig.y) / ray.dir.y;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.x >= rk.minimum.x && hitpos.x <= rk.maximum.x &&
						hitpos.z >= rk.minimum.z && hitpos.z <= rk.maximum.z &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			//minimum z
			if (ray.orig.z <= rk.minimum.z && ray.dir.z > 0.0f)
			{
				t = (rk.minimum.z - ray.orig.z) / ray.dir.z;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.x >= rk.minimum.x && hitpos.x <= rk.maximum.x &&
						hitpos.y >= rk.minimum.y && hitpos.y <= rk.maximum.y &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			//maximum z
			if (ray.orig.z >= rk.maximum.z && ray.dir.z < 0.0f)
			{
				t = (rk.maximum.z - ray.orig.z) / ray.dir.z;
				if (t >= 0)
				{
					hitpos = ray.orig + ray.dir * t;

					if (hitpos.x >= rk.minimum.x && hitpos.x <= rk.maximum.x &&
						hitpos.y >= rk.minimum.y && hitpos.y <= rk.maximum.y &&
						(!hit || t < lowt))
					{
						hit = true;
						lowt = t;
					}
				}
			}

			dist = lowt;
		}

		if (_dist)
			*_dist = dist;

		return hit;
	}

	inline bool Intersect(const Ray & ray, float * _dist, float & tu, float & tv, const Float3 & a, const Float3 & b, const Float3 & c)
	{
		/*
		p = o + dt;
		p = sa + qb + rc;
		s + q + r = 1;

		==>

		a + (b-a)q + (c-a)r = o + dt

		==>

		-dt + (b-a)q + (c-a)r = o - a

		==>

		| t |
		|-d, b - a, c - a|  | q | = | o - a |
		| r |

		==>

		|o - a, b - a, c - a|
		t = --------------------------
		|-d, b - a, c - a|

		|-d, o - a, c - a|
		q = --------------------------
		|-d, b - a, c - a|

		|-d, b - a, o - a|
		r = --------------------------
		|-d, b - a, c - a|
		*/

		float dist = FLT_MAX;

		Float3 ab = b - a;
		Float3 ac = c - a;

		Float3 cross = Float3::Cross(ray.dir, ac);
		float det = ab.dot(cross);

		Float3 ray2a = ray.orig - a;

		if (det < 0)
		{
			ray2a = a - ray.orig;
			det = -det;
		}

		//parallel
		if (det < 0.0001)
			return false;

		//q
		float q = ray2a.dot(cross);

		if (q < 0.0f || q > det)
			return false;

		//r
		cross = Float3::Cross(ray2a, ab);
		float r = ray.dir.dot(cross);

		if (r < 0.0f || r + q > det)
			return false;

		//t
		float t = ac.dot(cross);

		det = 1.0f / det;
		dist = t * det;
		tu = q * det;
		tv = r * det;

		if (_dist)
			*_dist = dist;

		return dist >= 0;
	}

	inline Float3 CalcuNormal(const Float3 & a, const Float3 & b, const Float3 & c)
	{
		Float3 ab = b - a;
		Float3 ac = c - a;

		Float3 n = Float3::Cross(ab, ac);
		n.normalize();

		return n;
	}

}