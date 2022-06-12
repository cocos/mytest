#ifndef __VEC4_H__
#define __VEC4_H__

namespace LFX {
	/**
	\brief 1x4 Vector (row vector)
	*/
	template <typename T>
	class Vec4T
	{
	public:

#pragma pack(push, 1)
		union
		{
			struct
			{
				T x, y, z, w;
			};
			T m[4];
		};
#pragma pack(pop)

	public:
		Vec4T();
		Vec4T(const T x, const T y, const T z, const T w);
		Vec4T(const T value);

	public:
		Vec4T& operator += (const Vec4T &rhs);
		Vec4T& operator += (const T value);
		Vec4T& operator -= (const Vec4T &rhs);
		Vec4T& operator -= (const T value);
		Vec4T& operator *= (const Vec4T &rhs);
		Vec4T& operator *= (const T value);
		Vec4T& operator /= (const Vec4T &rhs);
		Vec4T& operator /= (const T value);
		T& operator [] (int index);
		const T& operator [] (int index) const;

	public:
		T				dot(const Vec4T &rhs) const;
		T				len() const;
		T				lenSqr() const;
		Vec4T<T>		midPoint(const Vec4T& vec) const;
		const T*		ptr() const;
		T*				ptr();
		void			set(T _x, T _y, T _z, T _w = 0);
		void			set(T value);
		void			set(T *p);
		void			normalize();
		T				normalizeWithLen();
		void			saturate();
		void			clampZero();
		void			clampOne();

	public:
		static T		Dot(const Vec4T &a, const Vec4T &b);
		static Vec4T<T>	Lerp(const Vec4T &a, const Vec4T &b, const T t);
		static Vec4T<T>	Max(const Vec4T &a, const Vec4T &b);
		static Vec4T<T>	Min(const Vec4T &a, const Vec4T &b);
		static Vec4T<T>	Inverse(const Vec4T &a);
		static Vec4T<T>	Normalize(const Vec4T &a);
		static Vec4T<T>	NormalizeWithLen(const Vec4T &a, T& len);
		static Vec4T<T>	Saturate(const Vec4T &a);
	};

	template <typename T> Vec4T<T> operator + (const Vec4T<T> &rhs);
	template <typename T> Vec4T<T> operator - (const Vec4T<T> &rhs);
	template <typename T> Vec4T<T> operator + (const Vec4T<T> &a, const Vec4T<T> &b);
	template <typename T> Vec4T<T> operator - (const Vec4T<T> &a, const Vec4T<T> &b);
	template <typename T> Vec4T<T> operator * (const T f, const Vec4T<T> &v);
	template <typename T> Vec4T<T> operator * (const Vec4T<T> &v, const T f);
	template <typename T> Vec4T<T> operator * (const Vec4T<T> a, const Vec4T<T> &b);
	template <typename T> Vec4T<T> operator / (const Vec4T<T> &a, const T f);
	template <typename T> Vec4T<T> operator / (const Vec4T<T> a, const Vec4T<T> &b);

#include "vec4.inl"

	typedef Vec4T<int> Int4;
	typedef Vec4T<float> Float4;
}

#endif