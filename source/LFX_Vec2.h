#ifndef __VEC2_H__
#define __VEC2_H__

namespace LFX {

	// 1x2 Vector (row vector)
	template <typename T>
	class Vec2T
	{
	public:

#pragma pack(push, 1)
		union
		{
			struct
			{
				T x, y;
			};

			T m[2];
		};
#pragma pack(pop)

	public:
		Vec2T();
		Vec2T(const T x, const T y);

	public:
		Vec2T& operator += (const Vec2T &rhs);
		Vec2T& operator -= (const Vec2T &rhs);
		Vec2T& operator *= (const T rhs);
		Vec2T& operator /= (const T rhs);
		T& operator [] (int index);
		const T& operator [] (int index) const;
		bool operator < (const Vec2T &rhs) const;
		bool operator <= (const Vec2T &rhs) const;
		bool operator > (const Vec2T &rhs) const;
		bool operator >= (const Vec2T &rhs) const;

	public:
		T				dot(const Vec2T &rhs) const;
		T				cross(const Vec2T &rhs) const;
		T				len() const;
		T				lenSqr() const;
		Vec2T<T>		midPoint(const Vec2T &vec) const;
		Vec2T<T>		perpendicular() const;
		const T*		ptr() const;
		T*				ptr();
		void			set(T _x, T _y);
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
		static T		Dot(const Vec2T &a, const Vec2T &b);
		static T		Cross(const Vec2T &a, const Vec2T &b);
		static Vec2T<T>	Lerp(const Vec2T &a, const Vec2T &b, const T t);
		static Vec2T<T>	Max(const Vec2T &a, const Vec2T &b);
		static Vec2T<T>	Min(const Vec2T &a, const Vec2T &b);
		static Vec2T<T>	Inverse(const Vec2T &a);
		static Vec2T<T>	Normalize(const Vec2T &a);
		static Vec2T<T>	NormalizeWithLen(const Vec2T &a, T& len);
		static Vec2T<T>	Saturate(const Vec2T &a);
	};

	template <typename T> Vec2T<T> operator + (const Vec2T<T> &rhs);
	template <typename T> Vec2T<T> operator - (const Vec2T<T> &rhs);
	template <typename T> Vec2T<T> operator + (const Vec2T<T> &a, const Vec2T<T> &b);
	template <typename T> Vec2T<T> operator - (const Vec2T<T> &a, const Vec2T<T> &b);
	template <typename T> Vec2T<T> operator * (const T f, const Vec2T<T> &v);
	template <typename T> Vec2T<T> operator * (const Vec2T<T> &v, const T f);
	template <typename T> Vec2T<T> operator * (const Vec2T<T> a, const Vec2T<T> &b);
	template <typename T> Vec2T<T> operator / (const Vec2T<T> &a, const T f);
	template <typename T> Vec2T<T> operator / (const T f, const Vec2T<T> &a);
	template <typename T> Vec2T<T> operator / (const Vec2T<T> a, const Vec2T<T> &b);

#include "LFX_Vec2.inl"

	typedef Vec2T<int> Int2;
	typedef Vec2T<float> Float2;
}

#endif