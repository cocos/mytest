template <typename T> inline 
Vec4T<T>::Vec4T()
{
	x = y = z = w = 0;
}

template <typename T> inline 
Vec4T<T>::Vec4T(const T _x, const T _y, const T _z, const T _w)
: x(_x)
, y(_y)
, z(_z)
, w(_w)
{
}

template <typename T> inline 
Vec4T<T>::Vec4T(const T value)
: x(value)
, y(value)
, z(value)
, w(value)
{
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator += (const Vec4T &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator += (const T value)
{
	x += value;
	y += value;
	z += value;
	w += value;
	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator -= (const Vec4T &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;

	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator -= (const T value)
{
	x -= value;
	y -= value;
	z -= value;
	w -= value;

	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator *= (const Vec4T &rhs)
{
	x *= rhs.x;
	y *= rhs.y;
	z *= rhs.z;
	w *= rhs.w;

	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator *= (const T value)
{
	x *= value;
	y *= value;
	z *= value;
	w *= value;

	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator /= (const Vec4T &rhs)
{
	x /= rhs.x;
	y /= rhs.y;
	z /= rhs.z;
	w /= rhs.w;

	return *this;
}

template <typename T> inline 
Vec4T<T>& Vec4T<T>::operator /= (const T value)
{
	x /= value;
	y /= value;
	z /= value;
	w /= value;

	return *this;
}

template <typename T> inline 
T& Vec4T<T>::operator [] (int index)
{
	return m[index];
}

template <typename T> inline 
const T& Vec4T<T>::operator [] (int index) const
{
	return m[index];
}

template <typename T> inline 
T* Vec4T<T>::ptr()
{
	return &x;
}

template <typename T> inline 
const T* Vec4T<T>::ptr() const
{
	return &x;
}

template <typename T> inline 
void Vec4T<T>::set(T _x, T _y, T _z, T _w)
{
	this->x = _x;
	this->y = _y;
	this->z = _z;
	this->w = _w;
}

template <typename T> inline 
void Vec4T<T>::set(T value)
{
	this->x = value;
	this->y = value;
	this->z = value;
	this->w = value;
}

template <typename T> inline 
void Vec4T<T>::set(T *p)
{
	this->x = p[0];
	this->y = p[1];
	this->z = p[2];
	this->w = p[3];
}

template <typename T> inline 
T Vec4T<T>::dot(const Vec4T &rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
}

template <typename T> inline 
T Vec4T<T>::len() const
{
	return sqrt(x * x + y * y + z * z + w * w);
}

template <typename T> inline 
T Vec4T<T>::lenSqr() const
{
	return (x * x + y * y + z * z + w * w);
}

template <typename T> inline 
void Vec4T<T>::normalize()
{
	T length = len();
	x /= length;
	y /= length;
	z /= length;
	w /= length;
}

template <typename T> inline 
T Vec4T<T>::normalizeWithLen()
{
	T length = len();
	x /= length;
	y /= length;
	z /= length;
	w /= length;

	return length;
}

template <typename T> inline 
void Vec4T<T>::saturate()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;
	if ( z > 1 ) z = 1;
	if ( w > 1 ) w = 1;

	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( z < 0 ) z = 0;
	if ( w < 0 ) w = 0;
}

template <typename T> inline 
void Vec4T<T>::clampZero()
{
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( z < 0 ) z = 0;
	if ( w < 0 ) w = 0;
}

template <typename T> inline 
void Vec4T<T>::clampOne()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;
	if ( z > 1 ) z = 1;
	if ( w > 1 ) w = 1;
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::midPoint(const Vec4T& vec) const
{
	return Vec4T((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f, 1.0);
}

template <typename T> inline 
T Vec4T<T>::Dot(const Vec4T &a, const Vec4T &b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w);
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Lerp(const Vec4T &a, const Vec4T &b, const T t)
{
	return a + (b - a) * t;
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Max(const Vec4T &a, const Vec4T &b)
{
	return Vec4T((a.x > b.x ? a.x : b.x),
		(a.y > b.y ? a.y : b.y),
		(a.z > b.z ? a.z : b.z),
		(a.w > b.w ? a.w : b.w));
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Min(const Vec4T &a, const Vec4T &b)
{
	return Vec4T((a.x < b.x ? a.x : b.x),
		(a.y < b.y ? a.y : b.y),
		(a.z < b.z ? a.z : b.z),
		(a.w < b.w ? a.w : b.w));
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Inverse(const Vec4T &a)
{
	Vec4T<T> outVec = a;
	outVec.inverse();
	return outVec;
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Normalize(const Vec4T &a)
{
	Vec4T<T> outVec = a;
	outVec.normalize();
	return outVec;
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::NormalizeWithLen(const Vec4T &a, T& len)
{
	Vec4T<T> outVec = a;
	len = outVec.normalizeWithLen();
	return outVec;
}

template <typename T> inline 
Vec4T<T> Vec4T<T>::Saturate(const Vec4T &a)
{
	Vec4T<T> outVec = a;
	outVec.saturate();
	return outVec;
}

template <typename T> inline 
Vec4T<T> operator + (const Vec4T<T> &rhs)
{
	return rhs;
}

template <typename T> inline 
Vec4T<T> operator - (const Vec4T<T> &rhs)
{
	return Vec4T<T>(-rhs.x, -rhs.y, -rhs.z, -rhs.w);
}

template <typename T> inline 
Vec4T<T> operator + (const Vec4T<T> &a, const Vec4T<T> &b)
{
	return Vec4T<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template <typename T> inline 
Vec4T<T> operator - (const Vec4T<T> &a, const Vec4T<T> &b)
{
	return Vec4T<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template <typename T> inline 
Vec4T<T> operator * (const T f, const Vec4T<T> &v)
{
	return Vec4T<T>(f * v.x, f * v.y, f * v.z, f * v.w);
}

template <typename T> inline 
Vec4T<T> operator * (const Vec4T<T> &v, const T f)
{
	return Vec4T<T>(f * v.x, f * v.y, f * v.z, f * v.w);
}

template <typename T> inline 
Vec4T<T> operator * (const Vec4T<T> a, const Vec4T<T> &b)
{
	return Vec4T<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

template <typename T> inline 
Vec4T<T> operator / (const Vec4T<T> &a, const T f)
{
	return Vec4T<T>(a.x / f, a.y / f, a.z / f, a.w / f);
}

template <typename T> inline 
Vec4T<T> operator / (const T f, const Vec4T<T> &a)
{
	return Vec4T<T>(f / a.x, f / a.y, f / a.z, f / a.w);
}

template <typename T> inline 
Vec4T<T> operator / (const Vec4T<T> a, const Vec4T<T> &b)
{
	return Vec4T<T>(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}

template <typename T> inline
bool operator == (const Vec4T<T> a, const Vec4T<T>& b)
{
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template <typename T> inline
bool operator != (const Vec4T<T> a, const Vec4T<T>& b)
{
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}