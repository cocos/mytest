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

	struct Mat4
	{
		float _11, _12, _13, _14;
		float _21, _22, _23, _24;
		float _31, _32, _33, _34;
		float _41, _42, _43, _44;

		void SetXBasis(const Float3& x)
		{
			_11 = x.x;
			_12 = x.y;
			_13 = x.z;
			_14 = 0;
		}

		void SetYBasis(const Float3& y)
		{
			_21 = y.x;
			_22 = y.y;
			_23 = y.z;
			_24 = 0;
		}

		void SetZBasis(const Float3& z)
		{
			_31 = z.x;
			_32 = z.y;
			_33 = z.z;
			_34 = 0;
		}

		void SetTranslate(const Float3& t)
		{
			_41 = t.x;
			_42 = t.y;
			_43 = t.z;
			_44 =1;
		}

		static Float3 Transform(const Float3& v, const Mat4& m)
		{
			Float3 r;
			r.x = v.x * m._11 + v.y * m._21 + v.z * m._31 + m._41;
			r.y = v.x * m._12 + v.y * m._22 + v.z * m._32 + m._42;
			r.z = v.x * m._13 + v.y * m._23 + v.z * m._33 + m._43;
			return r;
		}

		static Float3 TransformN(const Float3& v, const Mat4& m)
		{
			Float3 r;
			r.x = v.x * m._11 + v.y * m._21 + v.z * m._31;
			r.y = v.x * m._12 + v.y * m._22 + v.z * m._32;
			r.z = v.x * m._13 + v.y * m._23 + v.z * m._33;
			return r;
		}
	};

}