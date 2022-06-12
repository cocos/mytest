template <typename T> inline 
Vec3T<T>::Vec3T()
: x(0)
, y(0)
, z(0)
{
}

template <typename T> inline 
Vec3T<T>::Vec3T(const T _x, const T _y, const T _z)
: x(_x)
, y(_y)
, z(_z)
{
}

template <typename T> inline 
Vec3T<T>& Vec3T<T>::operator += (const Vec3T &rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

template <typename T> inline 
Vec3T<T>& Vec3T<T>::operator -= (const Vec3T &rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

template <typename T> inline 
Vec3T<T>& Vec3T<T>::operator *= (const T value)
{
	x *= value;
	y *= value;
	z *= value;
	return *this;
}

template <typename T> inline 
Vec3T<T>& Vec3T<T>::operator /= (const T value)
{
	x /= value;
	y /= value;
	z /= value;
	return *this;
}

template <typename T> inline 
T& Vec3T<T>::operator [] (int index)
{
	return m[index];
}

template <typename T> inline 
const T& Vec3T<T>::operator [] (int index) const
{
	return m[index];
}

template <typename T> inline 
bool Vec3T<T>::operator < (const Vec3T &rhs) const
{
	if( x < rhs.x && y < rhs.y && z < rhs.z)
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec3T<T>::operator <= (const Vec3T &rhs) const
{
	if( x <= rhs.x && y <= rhs.y && z <= rhs.z)
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec3T<T>::operator > (const Vec3T &rhs) const
{
	if( x > rhs.x && y > rhs.y && z > rhs.z)
		return true;
	else
		return false;
}

template <typename T> inline 
bool Vec3T<T>::operator >= (const Vec3T &rhs) const
{
	if( x >= rhs.x && y >= rhs.y && z >= rhs.z)
		return true;
	else
		return false;
}

template <typename T> inline 
T* Vec3T<T>::ptr()
{
	return &x;
}

template <typename T> inline 
const T* Vec3T<T>::ptr() const
{
	return &x;
}

template <typename T> inline 
void Vec3T<T>::zero()
{
	x = 0;
	y = 0;
	z = 0;
}

template <typename T> inline 
void Vec3T<T>::one()
{
	x = 1;
	y = 1;
	z = 1;
}

template <typename T> inline 
void Vec3T<T>::set(T _x, T _y, T _z)
{
	this->x = _x;
	this->y = _y;
	this->z = _z;
}

template <typename T> inline 
void Vec3T<T>::set(T *p)
{
	this->x = p[0];
	this->y = p[1];
	this->z = p[2];
}

template <typename T> inline 
T Vec3T<T>::dot(const Vec3T& rhs) const
{
	return (x * rhs.x + y * rhs.y + z * rhs.z);
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::cross(const Vec3T& rhs) const
{
	return Vec3T<T>(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

template <typename T> inline 
void Vec3T<T>::inverse()
{
	x = 1.0f / x;
	y = 1.0f / y;
	z = 1.0f / z;
}

template <typename T> inline
T Vec3T<T>::len() const
{
	return sqrt(x * x + y * y + z * z);
}

template <typename T> inline 
T Vec3T<T>::lenSqr() const
{
	return (x * x + y * y + z * z);
}

template <typename T> inline 
void Vec3T<T>::normalize()
{
	T length = len();
	x /= length;
	y /= length;
	z /= length;
}

template <typename T> inline 
T Vec3T<T>::normalizeWithLen()
{
	T length = len();

	x /= length;
	y /= length;
	z /= length;

	return length;
}

template <typename T> inline 
void Vec3T<T>::saturate()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;
	if ( z > 1 ) z = 1;

	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( z < 0 ) z = 0;
}

template <typename T> inline 
void Vec3T<T>::clampZero()
{
	if ( x < 0 ) x = 0;
	if ( y < 0 ) y = 0;
	if ( z < 0 ) z = 0;
}

template <typename T> inline 
void Vec3T<T>::clampOne()
{
	if ( x > 1 ) x = 1;
	if ( y > 1 ) y = 1;
	if ( z > 1 ) z = 1;

	return *this;
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::midPoint(const Vec3T& vec) const
{
	return Vec3T((x + vec.x) * 0.5f, (y + vec.y) * 0.5f, (z + vec.z) * 0.5f);
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::perpendicular() const
{
	Vec3T<T> outVec = this->cross(Vec3T<T>(1, 0, 0));

	// Check length
	if(outVec.lenSqr() == 0.0)
	{
		/* This vector is the Y axis multiplied by a scalar, so we have
		   to use another axis.
		*/
		outVec = this->cross(Vec3T<T>(0, 1, 0));
	}
	outVec.normalize();
	return outVec;
}

template <typename T> inline 
T Vec3T<T>::Dot(const Vec3T &a, const Vec3T &b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Cross(const Vec3T &a, const Vec3T &b)
{
	return Vec3T(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Lerp(const Vec3T &a, const Vec3T &b, const T t)
{
	return a + (b - a) * t;
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Max(const Vec3T &a, const Vec3T &b)
{
	return Vec3T((a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y), (a.z > b.z ? a.z : b.z));
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Min(const Vec3T &a, const Vec3T &b)
{
	return Vec3T((a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y), (a.z < b.z ? a.z : b.z));
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Inverse(const Vec3T &a)
{
	Vec3T<T> outVec = a;
	outVec.inverse();
	return outVec;
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Normalize(const Vec3T &a)
{
	Vec3T<T> outVec = a;
	outVec.normalize();
	return outVec;
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::NormalizeWithLen(const Vec3T &a, T& len)
{
	Vec3T<T> outVec = a;
	len = outVec.normalizeWithLen();
	return outVec;
}

template <typename T> inline 
Vec3T<T> Vec3T<T>::Saturate(const Vec3T &a)
{
	Vec3T<T> outVec = a;
	outVec.saturate();
	return outVec;
}

template <typename T> inline 
Vec3T<T> operator + (const Vec3T<T> &rhs)
{
	return rhs;
}

template <typename T> inline 
Vec3T<T> operator - (const Vec3T<T> &rhs)
{
	return Vec3T<T>(-rhs.x, -rhs.y, -rhs.z);
}

template <typename T> inline 
Vec3T<T> operator + (const Vec3T<T> &a, const Vec3T<T> &b)
{
	return Vec3T<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename T> inline 
Vec3T<T> operator - (const Vec3T<T> &a, const Vec3T<T> &b)
{
	return Vec3T<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename T> inline 
Vec3T<T> operator * (const T f, const Vec3T<T> &v)
{
	return Vec3T<T>(f * v.x, f * v.y, f * v.z);
}

template <typename T> inline 
Vec3T<T> operator * (const Vec3T<T> &v, const T f)
{
	return Vec3T<T>(f * v.x, f * v.y, f * v.z);
}

template <typename T> inline 
Vec3T<T> operator * (const Vec3T<T> a, const Vec3T<T> &b)
{
	return Vec3T<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template <typename T> inline 
Vec3T<T> operator / (const Vec3T<T> &a, const T f)
{
	return Vec3T<T>(a.x / f, a.y / f, a.z / f);
}

template <typename T> inline 
Vec3T<T> operator / (const T f, const Vec3T<T> &a)
{
	return Vec3T<T>(f / a.x, f / a.y, f / a.z);
}

template <typename T> inline 
Vec3T<T> operator / (const Vec3T<T> a, const Vec3T<T> &b)
{
	return Vec3T<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}