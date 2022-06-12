#ifndef __VEC3_H__
#define __VEC3_H__

namespace LFX {

	// 1x3 Vector (row vector)
	template <typename T>
	class Vec3T
	{
	public:

#pragma pack(push, 1)
		union
		{
			struct
			{
				T x, y, z;
			};

			T m[3];
		};
#pragma pack(pop)

	public:
		Vec3T();
		Vec3T(const T x, const T y, const T z);

	public:
		Vec3T& operator += (const Vec3T &rhs);
		Vec3T& operator -= (const Vec3T &rhs);
		Vec3T& operator *= (const T value);
		Vec3T& operator /= (const T value);
		T& operator [] (int index);
		const T& operator [] (int index) const;
		bool operator < (const Vec3T &rhs) const;
		bool operator <= (const Vec3T &rhs) const;
		bool operator > (const Vec3T &rhs) const;
		bool operator >= (const Vec3T &rhs) const;

	public:
		T				dot(const Vec3T& rhs) const;
		Vec3T<T>		cross(const Vec3T& rhs) const;
		T				len() const;
		T				lenSqr() const;
		bool			isZeroLength() const;
		Vec3T<T>		midPoint(const Vec3T& vec) const;
		Vec3T<T>		randomDeviant(const T& angle, const Vec3T& up /*= Vec3T<T>(0, 0, 0)*/) const;
		Vec3T<T>		perpendicular() const;
		const T*		ptr() const;
		T*				ptr();
		void			set(T _x, T _y, T _z);
		void			set(T *p);
		void			zero();
		void			one();
		void			inverse();
		void			normalize();
		T				normalizeWithLen();
		void			saturate();
		void			clampZero();
		void			clampOne();

	public:
		static T		Dot(const Vec3T &a, const Vec3T &b);
		static Vec3T<T>	Cross(const Vec3T &a, const Vec3T &b);
		static Vec3T<T>	Lerp(const Vec3T &a, const Vec3T &b, const T t);
		static Vec3T<T>	Max(const Vec3T &a, const Vec3T &b);
		static Vec3T<T>	Min(const Vec3T &a, const Vec3T &b);
		static Vec3T<T>	Inverse(const Vec3T &a);
		static Vec3T<T>	Normalize(const Vec3T &a);
		static Vec3T<T>	NormalizeWithLen(const Vec3T &a, T& len);
		static Vec3T<T>	Saturate(const Vec3T &a);
	};

	template <typename T> Vec3T<T> operator + (const Vec3T<T> &rhs);
	template <typename T> Vec3T<T> operator - (const Vec3T<T> &rhs);
	template <typename T> Vec3T<T> operator + (const Vec3T<T> &a, const Vec3T<T> &b);
	template <typename T> Vec3T<T> operator - (const Vec3T<T> &a, const Vec3T<T> &b);
	template <typename T> Vec3T<T> operator * (const T f, const Vec3T<T> &v);
	template <typename T> Vec3T<T> operator * (const Vec3T<T> &v, const T f);
	template <typename T> Vec3T<T> operator * (const Vec3T<T> a, const Vec3T<T> &b);
	template <typename T> Vec3T<T> operator / (const Vec3T<T> &a, const T f);
	template <typename T> Vec3T<T> operator / (const T f, const Vec3T<T> &a);
	template <typename T> Vec3T<T> operator / (const Vec3T<T> a, const Vec3T<T> &b);

	template <typename T> Vec3T<T> operator + (const Vec3T<T> &v, const T f)
	{
		return Vec3T<T>(v.x + f, v.y + f, v.z + f);
	}

	template <typename T> Vec3T<T> operator - (const Vec3T<T> &v, const T f)
	{
		return Vec3T<T>(v.x - f, v.y - f, v.z - f);
	}

#include "vec3.inl"

	typedef Vec3T<int> Int3;
	typedef Vec3T<float> Float3;
}

#endif