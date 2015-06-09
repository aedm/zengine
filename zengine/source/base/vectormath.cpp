#include <include/base/vectormath.h>
#include <math.h>

Vec2::Vec2()
{
}

Vec2::Vec2(float X, float Y)
{
	this->x = X;
	this->y = Y;
}

Vec2 Vec2::operator- (const Vec2& op) const
{
	return Vec2(x - op.x, y - op.y);
}

Vec2 Vec2::operator+ (const Vec2& op) const
{
	return Vec2(x + op.x, y + op.y);
}

Vec2 Vec2::operator/ (const Vec2& op) const
{
	return Vec2(x / op.x, y / op.y);
}

Vec2 Vec2::operator* (const Vec2& op) const
{
	return Vec2(x * op.x, y * op.y);
}

Vec2 Vec2::operator* (float F) const
{
	return Vec2(x * F, y * F);
}

Vec2 Vec2::operator/ (float F) const
{
	return *this * (1.0f / F);
}

Vec2& Vec2::operator*=( float F )
{
	x *= F;
	y *= F;
	return *this;
}

float Vec2::Length() const
{
	return sqrtf(x*x + y*y);
}

Vec3::Vec3()
{
}

Vec3::Vec3(float X, float Y, float Z)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
}

float Vec3::Dot(const Vec3& op) const
{
	return x * op.x + y * op.y + z * op.z;
}

Vec3 Vec3::operator* (float F) const
{
	return Vec3(x * F, y * F, z * F);
}

Vec3 Vec3::operator- (const Vec3& op) const
{
	return Vec3(x - op.x, y - op.y, z - op.z);
}

Vec3 Vec3::operator+ (const Vec3& op) const
{
	return Vec3(x + op.x, y + op.y, z + op.z);
}

Vec3 Vec3::operator- () const
{
	return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator/( float F ) const
{
	return Vec3(x/F, y/F, z/F);
}

Vec3 Vec3::Cross(const Vec3& op) const
{
	return Vec3(y * op.z - z * op.y, z * op.x - x * op.z, x * op.y - y * op.x);
}

float Vec3::LengthSquared() const
{
	return x*x + y*y + z*z;
}

float Vec3::Length() const
{
	return sqrtf(x*x + y*y + z*z);
}

Vec3 Vec3::Normal() const
{
	return *this / Length();
}

Vec3& Vec3::Normalize()
{
	*this /= Length();
	return *this;
}

Vec3& Vec3::operator/=( float F )
{
	x /= F;
	y /= F;
	z /= F;
	return *this;
}

Vec3& Vec3::operator*=( float F )
{
	x *= F;
	y *= F;
	z *= F;
	return *this;
}

Vec3& Vec3::operator+=(const Vec3& op)
{
	x += op.x;
	y += op.y;
	z += op.z;
	return *this;
}

Vec4::Vec4()
{
}

Vec4::Vec4(float X, float Y, float Z, float W)
{
	this->x = X;
	this->y = Y;
	this->z = Z;
	this->w = W;
}

Vec4::Vec4( Vec3 V, float W )
{
	this->x = V.x;
	this->y = V.y;
	this->z = V.z;
	this->w = W;
}

float Vec4::Dot( const Vec4& V ) const
{
	return x*V.x + y*V.y + z*V.z + w*V.w;
}

Vec4 Vec4::operator-( const Vec4& V ) const
{
	return Vec4(x-V.x, y-V.y, z-V.z, w-V.w);
}

bool Vec4::operator == ( const Vec4& V ) const
{
	return x==V.x && y==V.y && z==V.z && w==V.w;
}

Vec3 Vec4::XYZ()
{
	return Vec3(x, y, z);
}

Matrix::Matrix()
{
}

float& Matrix::operator() (UINT Y, UINT X)
{
	return m[Y * 4 + X];
}

Matrix Matrix::Translate(const Vec3& TranslateVector)
{
	Matrix matrix;
	matrix.m[ 0] = 1;	matrix.m[ 1] = 0;	matrix.m[ 2] = 0;	matrix.m[ 3] = TranslateVector.x;
	matrix.m[ 4] = 0;	matrix.m[ 5] = 1;	matrix.m[ 6] = 0;	matrix.m[ 7] = TranslateVector.y;
	matrix.m[ 8] = 0;	matrix.m[ 9] = 0;	matrix.m[10] = 1;	matrix.m[11] = TranslateVector.z;
	matrix.m[12] = 0;	matrix.m[13] = 0;	matrix.m[14] = 0;	matrix.m[15] = 1;

	return matrix;
}

Matrix Matrix::Rotate(float Angle, const Vec3& Axis)
{
	Matrix rot;
	float sn = sin(Angle);
	float cs = cos(Angle);
	float nc = 1-cs;
	rot.m[0]  = nc * (Axis.x * Axis.x) + cs;
	rot.m[1]  = nc * (Axis.y * Axis.x) - sn * Axis.z;
	rot.m[2]  = nc * (Axis.z * Axis.x) + sn * Axis.y;
	rot.m[3]  = 0;
	rot.m[4]  = nc * (Axis.x * Axis.y) + sn * Axis.z;
	rot.m[5]  = nc * (Axis.y * Axis.y) + cs;
	rot.m[6]  = nc * (Axis.z * Axis.y) - sn * Axis.x;
	rot.m[7]  = 0;
	rot.m[8]  = nc * (Axis.x * Axis.z) - sn * Axis.y;
	rot.m[9]  = nc * (Axis.y * Axis.z) + sn * Axis.x;
	rot.m[10] = nc * (Axis.z * Axis.z) + cs;
	rot.m[11]  = 0;
	rot.m[12]  = 0;
	rot.m[13]  = 0;
	rot.m[14]  = 0;
	rot.m[15]  = 1;
	return rot;
}

Matrix Matrix::Rotate( const Quaternion& Q )
{
	/// http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm

	Matrix rot;

	float sqw = Q.s * Q.s;
	float sqx = Q.vx * Q.vx;
	float sqy = Q.vy * Q.vy;
	float sqz = Q.vz * Q.vz;

	rot.m[ 0] =  sqx - sqy - sqz + sqw;
	rot.m[ 5] = -sqx + sqy - sqz + sqw;
	rot.m[10] = -sqx - sqy + sqz + sqw;

	float tmp1 = Q.vx * Q.vy;
	float tmp2 = Q.vz * Q.s;
	rot.m[ 1] = 2.0f * (tmp1 - tmp2);
	rot.m[ 4] = 2.0f * (tmp1 + tmp2);

	tmp1 = Q.vx * Q.vz;
	tmp2 = Q.vy * Q.s;
	rot.m[ 2] = 2.0f * (tmp1 + tmp2);
	rot.m[ 8] = 2.0f * (tmp1 - tmp2);

	tmp1 = Q.vy * Q.vz;
	tmp2 = Q.vx * Q.s;
	rot.m[ 6] = 2.0f * (tmp1 - tmp2);  
	rot.m[ 9] = 2.0f * (tmp1 + tmp2);

	rot.m[3] = rot.m[7] = rot.m[11] = rot.m[12] = rot.m[13] = rot.m[14] = 0;
	rot.m[15] = 1;
	return rot;
}

Matrix Matrix::Scale( const Vec3& ScaleVector )
{
	Matrix matrix;
	matrix.m[ 0] = ScaleVector.x;	matrix.m[ 1] = 0;	matrix.m[ 2] = 0;	matrix.m[ 3] = 0;
	matrix.m[ 4] = 0;	matrix.m[ 5] = ScaleVector.y;	matrix.m[ 6] = 0;	matrix.m[ 7] = 0;
	matrix.m[ 8] = 0;	matrix.m[ 9] = 0;	matrix.m[10] = ScaleVector.z;	matrix.m[11] = 0;
	matrix.m[12] = 0;	matrix.m[13] = 0;	matrix.m[14] = 0;				matrix.m[15] = 1;

	return matrix;
}

Matrix& Matrix::operator*=( const Matrix& M )
{
	Vec4 col[4];
	for (UINT x=0; x<4; x++)
	{
		col[x] = Vec4(M.m[x+0*4], M.m[x+1*4], M.m[x+2*4], M.m[x+3*4]);
	}

	for (UINT y=0; y<4; y++)
	{
		Vec4 row(m[y*4+0], m[y*4+1], m[y*4+2], m[y*4+3]);
		for (UINT x=0; x<4; x++)
		{
			m[y*4+x] = row.Dot(col[x]);
		}
	}

	return *this;
}

Matrix Matrix::operator * (const Matrix& M) const
{
	Matrix matrix = *this;
	matrix *= M;
	return matrix;
}

Vec4 Matrix::operator * (const Vec4& V) const
{
	Vec4 ret;
	ret.x = m[ 0] * V.x + m[ 1] * V.y + m[ 2] * V.z + m[ 3] * V.w;
	ret.y = m[ 4] * V.x + m[ 5] * V.y + m[ 6] * V.z + m[ 7] * V.w;
	ret.z = m[ 8] * V.x + m[ 9] * V.y + m[10] * V.z + m[11] * V.w;
	ret.w = m[12] * V.x + m[13] * V.y + m[14] * V.z + m[15] * V.w;
	return ret;
}

void Matrix::LoadIdentity()
{
	m[ 0] = 1;	m[ 1] = 0;	m[ 2] = 0;	m[ 3] = 0;	
	m[ 4] = 0;	m[ 5] = 1;	m[ 6] = 0;	m[ 7] = 0;	
	m[ 8] = 0;	m[ 9] = 0;	m[10] = 1;	m[11] = 0;	
	m[12] = 0;	m[13] = 0;	m[14] = 0;	m[15] = 1;	
}

Matrix Matrix::Ortho( float x1, float y1, float x2, float y2 )
{
	float rx = 1.0f / (x2-x1);
	float ry = 1.0f / (y2-y1);
	Matrix m;
	m.m[ 0] = 2.0f*rx;	m.m[ 1] = 0;		m.m[ 2] = 0;	m.m[ 3] = -(x1+x2) * rx;
	m.m[ 4] = 0;		m.m[ 5] = -2.0f*ry;	m.m[ 6] = 0;	m.m[ 7] = (y1+y2) * ry;
	m.m[ 8] = 0;		m.m[ 9] = 0;		m.m[10] = -1;	m.m[11] = 0;
	m.m[12] = 0;		m.m[13] = 0;		m.m[14] = 0;	m.m[15] = 1;
	return m;
}

void Quaternion::FromEuler(float Roll, float Pitch, float Yaw)
{
	float sr,cr,sp,cp,sy,cy;
	float rr = 0.5f*Roll;
	float pp = 0.5f*Pitch;
	float yy = 0.5f*Yaw;
	sr = sinf(rr); cr = cosf(rr);
	sp = sinf(pp); cp = cosf(pp);
	sy = sinf(yy); cy = cosf(yy);

	vx = sr*cp*cy - cr*sp*sy;
	vy = cr*sp*cy + sr*cp*sy;
	vz = cr*cp*sy - sr*sp*cy;
	s = cr*cp*cy + sr*sp*sy;
}

void Quaternion::ToEuler(float& oRoll, float& oPitch, float& oYaw)
{
	float a = 2.0f  *  (s * vy - vx * vz);
	if (a < 1.0f)
	{
		if (-1.0f < a)
		{
			oRoll = atan2f(2.0f * (vy * vz + s * vx), 1.0f - 2.0f * (vx * vx + vy * vy));
			oPitch = asinf(a);
			oYaw = atan2f(2.0f * (vx * vy + s * vz), 1.0f - 2.0f * (vy * vy + vz * vz));
		} 
		else 
		{
			oRoll = -atan2f(2.0f * (vx * vy-s * vz), 1.0f - 2.0f * (vx * vx + vz * vz));
			oPitch = Pi / -2.0f;
			oYaw = 0.0f;
		}
	} 
	else 
	{
		oRoll = atan2f(2.0f * (vx * vy - s * vz), 1.0f - 2.0f * (vx * vx + vz * vz));
		oPitch = Pi / 2.0f;
		oYaw = 0.0f;
	}
}

Quaternion::Quaternion(Quaternion& Q1, Quaternion& Q2, float Ratio)
{
	float cosom = Q1.s * Q2.s + Q1.vx * Q2.vx + Q1.vy * Q2.vy + Q1.vz * Q2.vz;
	float sclp, sclq;
	if (cosom < 0.9999f)
	{
		float omega, sinom;
		omega = acos(cosom);
		sinom = sin(omega);
		sclp = sin((1.0f - Ratio) * omega) / sinom;
		sclq = sin(Ratio * omega) / sinom;
	} 
	else 
	{
		sclp = 1.0f - Ratio;
		sclq = Ratio;
	}

	s = Q1.s * sclp + Q2.s * sclq;
	vx = Q1.vx * sclp + Q2.vx* sclq;
	vy = Q1.vy * sclp + Q2.vy * sclq;
	vz = Q1.vz * sclp + Q2.vz * sclq;
}

Quaternion::Quaternion() {}

Quaternion::Quaternion( float _S, float _Vx, float _Vy, float _Vz ): s(_S), vx(_Vx), vy(_Vy), vz(_Vz) {}

Quaternion& Quaternion::operator *= (Quaternion& Q)
{
	float bs = s*Q.s - vx*Q.vx - vy*Q.vy - vz*Q.vz;
	float bx = s*Q.vx + vx*Q.s + vy*Q.vz - vz*Q.vy;
	float by = s*Q.vy + vy*Q.s + vz*Q.vx - vx*Q.vz;
	float bz = s*Q.vz + vz*Q.s + vx*Q.vy - vy*Q.vx;

	s = bs;
	vx = bx;
	vy = by;
	vz = bz;
	return *this;
}

Quaternion Quaternion::operator *(const Quaternion &Q)
{
	float bs = s*Q.s - vx*Q.vx - vy*Q.vy - vz*Q.vz;
	float bx = s*Q.vx + vx*Q.s + vy*Q.vz - vz*Q.vy;
	float by = s*Q.vy + vy*Q.s + vz*Q.vx - vx*Q.vz;
	float bz = s*Q.vz + vz*Q.s + vx*Q.vy - vy*Q.vx;
	return Quaternion(bs, bx, by, bz);	
}

Quaternion& Quaternion::operator = (const Quaternion& Q)
{
	s = Q.s;
	vx = Q.vx;
	vy = Q.vy;
	vz = Q.vz;
	return *this;
}

Quaternion Quaternion::Conjugate()
{
	return Quaternion(s, -vx, -vy, -vz);
}