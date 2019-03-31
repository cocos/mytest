
template <typename T> inline 
Vec2T<T>::Vec2T()
: x(0)
, y(0)
{
	// do nothing
}

template <typename T> inline 
Vec2T<T>::Vec2T(const T _x, const T _y)
: x(_x)
, y(_y)
{
}

template <typename T> inline 
Vec2T<T>& Vec2T<T>::operator += (const Vec2T &rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

template <typename T> inline 
Vec2T<T>& Vec2T<T>::operator -= (const Vec2T &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

template <typename T> inline 
Vec2T<T>& Vec2T<T>::operator *= (const T value)
{
	x *= value;
	y *= value;
	return *this;
}

template <typename T> inline 
Vec2T<T>& Vec2T<T>::operator /= (const T value)
{
	x /= value;
	y /= value;
	return *this;
}

template <typename T> inline 
T& Vec2T<T>::operator [] (int index)
{
	return m[index];
}

template <typename T> inline 
const T& Vec2T<T>::operator [] (int index) const
{
	return m[index];
}

template <typename T> inline 
bool Vec2T<T>::operator < (const Vec2T &rhs) const
{
	if( x < rhs.x && y < rhs.y )
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec2T<T>::operator <= (const Vec2T &rhs) const
{
	if( x <= rhs.x && y <= rhs.y )
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec2T<T>::operator > (const Vec2T &rhs) const
{
	if( x > rhs.x && y > rhs.y )
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec2T<T>::operator >= (const Vec2T &rhs) const
{
	if( x >= rhs.x && y >= rhs.y )
		return true;
	else
		return false;
}

template <typename T> inline 
T* Vec2T<T>::ptr()
{
	return &x;
}

template <typename T> inline 
const T* Vec2T<T>::ptr() const
{
	return &x;
}

template <typename T> inline 
void Vec2T<T>::zero()
{
	x = 0;
	y = 0;
}

template <typename T> inline 
void Vec2T<T>::one()
{
	x = 1;
	y = 1;
}

template <typename T> inline 
void Vec2T<T>::set(T _x, T _y)
{
	this->x = _x;
	this->y = _y;
}

template <typename T> inline 
void Vec2T<T>::set(T *p)
{
	this->x = p[0];
	this->y = p[1];
}

template <typename T> inline 
T Vec2T<T>::dot(const Vec2T &rhs) const
{
	return (x * rhs.x + y * rhs.y);
}

template <typename T> inline 
T Vec2T<T>::cross(const Vec2T &rhs) const
{
	return (x * rhs.y - y * rhs.x);
}

template <typename T> inline 
void Vec2T<T>::inverse()
{
	x = 1.0f / x;
	y = 1.0f / y;
}

template <typename T> inline 
T Vec2T<T>::len() const
{
	return sqrt(x*x + y*y);
}

template <typename T> inline 
T Vec2T<T>::lenSqr() const
{
	return (x * x + y * y);
}

template <typename T> inline 
void Vec2T<T>::normalize()
{
	T length = len();
	x /= length;
	y /= length;
}

template <typename T> inline 
T Vec2T<T>::normalizeWithLen()
{
	T length = len();
	x /= length;
	y /= length;
	return length;
}

template <typename T> inline 
void Vec2T<T>::saturate()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;

	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
}

template <typename T> inline 
void Vec2T<T>::clampZero()
{
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
}

template <typename T> inline 
void Vec2T<T>::clampOne()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::midPoint(const Vec2T& vec) const
{
	return Vec2T((x + vec.x) * 0.5f, (y + vec.y) * 0.5f);
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::perpendicular() const
{
	return Vec2T(-y, x);
}

template <typename T> inline 
T Vec2T<T>::Dot(const Vec2T &a, const Vec2T &b)
{
	return a.x * b.x + a.y * b.y;
}

template <typename T> inline 
T Vec2T<T>::Cross(const Vec2T &a, const Vec2T &b)
{
	return a.x * b.y - a.y * b.x;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Lerp(const Vec2T &a, const Vec2T &b, const T t)
{
	return a + (b - a) * t;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Max(const Vec2T &a, const Vec2T &b)
{
	return Vec2T((a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y));
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Min(const Vec2T &a, const Vec2T &b)
{
	return Vec2T((a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y));
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Inverse(const Vec2T &a)
{
	Vec2T<T> outVec = a;
	outVec.inverse();
	return outVec;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Normalize(const Vec2T &a)
{
	Vec2T<T> outVec = a;
	outVec.normalize();
	return outVec;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::NormalizeWithLen(const Vec2T &a, T& len)
{
	Vec2T<T> outVec = a;
	len = outVec.normalizeWithLen();
	return outVec;
}

template <typename T> inline 
Vec2T<T> Vec2T<T>::Saturate(const Vec2T &a)
{
	Vec2T<T> outVec = a;
	outVec.saturate();
	return outVec;
}

template <typename T> inline 
Vec2T<T> operator + (const Vec2T<T> &rhs)
{
	return rhs;
}

template <typename T> inline 
Vec2T<T> operator - (const Vec2T<T> &rhs)
{
	return Vec2T<T>(-rhs.x, -rhs.y);
}

template <typename T> inline 
Vec2T<T> operator + (const Vec2T<T> &a, const Vec2T<T> &b)
{
	return Vec2T<T>(a.x + b.x, a.y + b.y);
}

template <typename T> inline 
Vec2T<T> operator - (const Vec2T<T> &a, const Vec2T<T> &b)
{
	return Vec2T<T>(a.x - b.x, a.y - b.y);
}

template <typename T> inline 
Vec2T<T> operator * (const T f, const Vec2T<T> &v)
{
	return Vec2T<T>(f * v.x, f * v.y);
}

template <typename T> inline 
Vec2T<T> operator * (const Vec2T<T> &v, const T f)
{
	return Vec2T<T>(f * v.x, f * v.y);
}

template <typename T> inline 
Vec2T<T> operator * (const Vec2T<T> a, const Vec2T<T> &b)
{
	return Vec2T<T>(a.x * b.x, a.y * b.y);
}

template <typename T> inline 
Vec2T<T> operator / (const Vec2T<T> &a, const T f)
{
	return Vec2T<T>(a.x / f, a.y / f);
}

template <typename T> inline 
Vec2T<T> operator / (const T f, const Vec2T<T> &a)
{
	return Vec2T<T>(f / a.x, f / a.y);
}

template <typename T> inline 
Vec2T<T> operator / (const Vec2T<T> a, const Vec2T<T> &b)
{
	return Vec2T<T>(a.x / b.x, a.y / b.y);
}