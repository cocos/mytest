#pragma once

#include "LFX_Vec3.h"

namespace LFX {

	struct Aabb
	{
		Float3 minimum;
		Float3 maximum;

		Aabb() {}
		Aabb(const Float3 & vmin, const Float3 & vmax) : minimum(vmin), maximum(vmax) {}

		void Invalid()
		{
			maximum = Float3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			minimum = Float3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
		}

		bool Valid()
		{
			return
				minimum.x <= maximum.x &&
				minimum.y <= maximum.y &&
				minimum.z <= maximum.z;
		}

		Float3 Size() const
		{
			return maximum - minimum;
		}

		Float3 Center() const
		{
			return (minimum + maximum) * 0.5f;
		}

		Float3 Extend() const
		{
			return Size() * 0.5f;
		}

		void Merge(const Aabb & rk)
		{
			minimum.x = std::min<float>(minimum.x, rk.minimum.x);
			minimum.y = std::min<float>(minimum.y, rk.minimum.y);
			minimum.z = std::min<float>(minimum.z, rk.minimum.z);
			maximum.x = std::max<float>(maximum.x, rk.maximum.x);
			maximum.y = std::max<float>(maximum.y, rk.maximum.y);
			maximum.z = std::max<float>(maximum.z, rk.maximum.z);
		}

		void Merge(const Float3 & rk)
		{
			minimum.x = std::min<float>(minimum.x, rk.x);
			minimum.y = std::min<float>(minimum.y, rk.y);
			minimum.z = std::min<float>(minimum.z, rk.z);
			maximum.x = std::max<float>(maximum.x, rk.x);
			maximum.y = std::max<float>(maximum.y, rk.y);
			maximum.z = std::max<float>(maximum.z, rk.z);
		}

		void GetCorner(Float3 * points) const
		{
			points[0] = Float3(minimum.x, minimum.y, minimum.z);
			points[1] = Float3(minimum.x, maximum.y, minimum.z);
			points[2] = Float3(maximum.x, minimum.y, minimum.z);
			points[3] = Float3(maximum.x, maximum.y, minimum.z);

			points[4] = Float3(minimum.x, minimum.y, maximum.z);
			points[5] = Float3(minimum.x, maximum.y, maximum.z);
			points[6] = Float3(maximum.x, minimum.y, maximum.z);
			points[7] = Float3(maximum.x, maximum.y, maximum.z);
		}

		bool Contain(const Float3 & rk)
		{
			return
				(rk.x >= minimum.x && rk.x <= maximum.x) &&
				(rk.y >= minimum.y && rk.y <= maximum.y) &&
				(rk.z >= minimum.z && rk.z <= maximum.z);
		}

		bool Contain(const Aabb & rk)
		{
			return
				(rk.minimum.x >= minimum.x && rk.maximum.x <= maximum.x) &&
				(rk.minimum.y >= minimum.y && rk.maximum.y <= maximum.y) &&
				(rk.minimum.z >= minimum.z && rk.maximum.z <= maximum.z);
		}

		bool Intersect(const Aabb & rk) const
		{
			if (minimum.x >= rk.maximum.x ||
				minimum.y >= rk.maximum.y ||
				minimum.z >= rk.maximum.z ||
				maximum.x <= rk.minimum.x ||
				maximum.y <= rk.minimum.y ||
				maximum.z <= rk.minimum.z)
				return false;

			return true;
		}
	};
}

