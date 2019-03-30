#pragma once

#include "LFX_Vec3.h"

namespace LFX {

	struct Mat3
	{
		float _11, _12, _13;
		float _21, _22, _23;
		float _31, _32, _33;

		void SetXBasis(const Float3& x)
		{
			_11 = x.x;
			_12 = x.y;
			_13 = x.z;
		}

		void SetYBasis(const Float3& y)
		{
			_21 = y.x;
			_22 = y.y;
			_23 = y.z;
		}

		void SetZBasis(const Float3& z)
		{
			_31 = z.x;
			_32 = z.y;
			_33 = z.z;
		}

		static Float3 Transform(const Float3& v, const Mat3& m)
		{
			Float3 r;
			r.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
			r.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
			r.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
			return r;
		}
	};

}