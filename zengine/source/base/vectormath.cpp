#include <include/base/vectormath.h>
#include <math.h>

Vec2::Vec2()
{
}

Vec2::Vec2(float X, float Y)
{
	this->X = X;
	this->Y = Y;
}

Vec2 Vec2::operator- (const Vec2& op) const
{
	return Vec2(X - op.X, Y - op.Y);
}

Vec2 Vec2::operator+ (const Vec2& op) const
{
	return Vec2(X + op.X, Y + op.Y);
}

Vec2 Vec2::operator/ (const Vec2& op) const
{
	return Vec2(X / op.X, Y / op.Y);
}

Vec2 Vec2::operator* (const Vec2& op) const
{
	return Vec2(X * op.X, Y * op.Y);
}

Vec2 Vec2::operator* (float F) const
{
	return Vec2(X * F, Y * F);
}

Vec2 Vec2::operator/ (float F) const
{
	return *this * (1.0f / F);
}

Vec2& Vec2::operator*=( float F )
{
	X *= F;
	Y *= F;
	return *this;
}

float Vec2::Length() const
{
	return sqrtf(X*X + Y*Y);
}

Vec3::Vec3()
{
}

Vec3::Vec3(float X, float Y, float Z)
{
	this->X = X;
	this->Y = Y;
	this->Z = Z;
}

float Vec3::Dot(const Vec3& op) const
{
	return X * op.X + Y * op.Y + Z * op.Z;
}

Vec3 Vec3::operator* (float F) const
{
	return Vec3(X * F, Y * F, Z * F);
}

Vec3 Vec3::operator- (const Vec3& op) const
{
	return Vec3(X - op.X, Y - op.Y, Z - op.Z);
}

Vec3 Vec3::operator+ (const Vec3& op) const
{
	return Vec3(X + op.X, Y + op.Y, Z + op.Z);
}

Vec3 Vec3::operator- () const
{
	return Vec3(-X, -Y, -Z);
}

Vec3 Vec3::operator/( float F ) const
{
	return Vec3(X/F, Y/F, Z/F);
}

Vec3 Vec3::Cross(const Vec3& op) const
{
	return Vec3(Y * op.Z - Z * op.Y, Z * op.X - X * op.Z, X * op.Y - Y * op.X);
}

float Vec3::LengthSquared() const
{
	return X*X + Y*Y + Z*Z;
}

float Vec3::Length() const
{
	return sqrtf(X*X + Y*Y + Z*Z);
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
	X /= F;
	Y /= F;
	Z /= F;
	return *this;
}

Vec3& Vec3::operator*=( float F )
{
	X *= F;
	Y *= F;
	Z *= F;
	return *this;
}

Vec3& Vec3::operator+=(const Vec3& op)
{
	X += op.X;
	Y += op.Y;
	Z += op.Z;
	return *this;
}

Vec4::Vec4()
{
}

Vec4::Vec4(float X, float Y, float Z, float W)
{
	this->X = X;
	this->Y = Y;
	this->Z = Z;
	this->W = W;
}

Vec4::Vec4( Vec3 V, float W )
{
	this->X = V.X;
	this->Y = V.Y;
	this->Z = V.Z;
	this->W = W;
}

float Vec4::Dot( const Vec4& V ) const
{
	return X*V.X + Y*V.Y + Z*V.Z + W*V.W;
}

Vec4 Vec4::operator-( const Vec4& V ) const
{
	return Vec4(X-V.X, Y-V.Y, Z-V.Z, W-V.W);
}

bool Vec4::operator == ( const Vec4& V ) const
{
	return X==V.X && Y==V.Y && Z==V.Z && W==V.W;
}

Vec3 Vec4::XYZ()
{
	return Vec3(X, Y, Z);
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
	matrix.m[ 0] = 1;	matrix.m[ 1] = 0;	matrix.m[ 2] = 0;	matrix.m[ 3] = TranslateVector.X;
	matrix.m[ 4] = 0;	matrix.m[ 5] = 1;	matrix.m[ 6] = 0;	matrix.m[ 7] = TranslateVector.Y;
	matrix.m[ 8] = 0;	matrix.m[ 9] = 0;	matrix.m[10] = 1;	matrix.m[11] = TranslateVector.Z;
	matrix.m[12] = 0;	matrix.m[13] = 0;	matrix.m[14] = 0;	matrix.m[15] = 1;

	return matrix;
}

Matrix Matrix::Rotate(float Angle, const Vec3& Axis)
{
	Matrix rot;
	float sn = sin(Angle);
	float cs = cos(Angle);
	float nc = 1-cs;
	rot.m[0]  = nc * (Axis.X * Axis.X) + cs;
	rot.m[1]  = nc * (Axis.Y * Axis.X) - sn * Axis.Z;
	rot.m[2]  = nc * (Axis.Z * Axis.X) + sn * Axis.Y;
	rot.m[3]  = 0;
	rot.m[4]  = nc * (Axis.X * Axis.Y) + sn * Axis.Z;
	rot.m[5]  = nc * (Axis.Y * Axis.Y) + cs;
	rot.m[6]  = nc * (Axis.Z * Axis.Y) - sn * Axis.X;
	rot.m[7]  = 0;
	rot.m[8]  = nc * (Axis.X * Axis.Z) - sn * Axis.Y;
	rot.m[9]  = nc * (Axis.Y * Axis.Z) + sn * Axis.X;
	rot.m[10] = nc * (Axis.Z * Axis.Z) + cs;
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

	float sqw = Q.S * Q.S;
	float sqx = Q.Vx * Q.Vx;
	float sqy = Q.Vy * Q.Vy;
	float sqz = Q.Vz * Q.Vz;

	rot.m[ 0] =  sqx - sqy - sqz + sqw;
	rot.m[ 5] = -sqx + sqy - sqz + sqw;
	rot.m[10] = -sqx - sqy + sqz + sqw;

	float tmp1 = Q.Vx * Q.Vy;
	float tmp2 = Q.Vz * Q.S;
	rot.m[ 1] = 2.0f * (tmp1 - tmp2);
	rot.m[ 4] = 2.0f * (tmp1 + tmp2);

	tmp1 = Q.Vx * Q.Vz;
	tmp2 = Q.Vy * Q.S;
	rot.m[ 2] = 2.0f * (tmp1 + tmp2);
	rot.m[ 8] = 2.0f * (tmp1 - tmp2);

	tmp1 = Q.Vy * Q.Vz;
	tmp2 = Q.Vx * Q.S;
	rot.m[ 6] = 2.0f * (tmp1 - tmp2);  
	rot.m[ 9] = 2.0f * (tmp1 + tmp2);

	rot.m[3] = rot.m[7] = rot.m[11] = rot.m[12] = rot.m[13] = rot.m[14] = 0;
	rot.m[15] = 1;
	return rot;
}

Matrix Matrix::Scale( const Vec3& ScaleVector )
{
	Matrix matrix;
	matrix.m[ 0] = ScaleVector.X;	matrix.m[ 1] = 0;	matrix.m[ 2] = 0;	matrix.m[ 3] = 0;
	matrix.m[ 4] = 0;	matrix.m[ 5] = ScaleVector.Y;	matrix.m[ 6] = 0;	matrix.m[ 7] = 0;
	matrix.m[ 8] = 0;	matrix.m[ 9] = 0;	matrix.m[10] = ScaleVector.Z;	matrix.m[11] = 0;
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
	ret.X = m[ 0] * V.X + m[ 1] * V.Y + m[ 2] * V.Z + m[ 3] * V.W;
	ret.Y = m[ 4] * V.X + m[ 5] * V.Y + m[ 6] * V.Z + m[ 7] * V.W;
	ret.Z = m[ 8] * V.X + m[ 9] * V.Y + m[10] * V.Z + m[11] * V.W;
	ret.W = m[12] * V.X + m[13] * V.Y + m[14] * V.Z + m[15] * V.W;
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

	Vx = sr*cp*cy - cr*sp*sy;
	Vy = cr*sp*cy + sr*cp*sy;
	Vz = cr*cp*sy - sr*sp*cy;
	S = cr*cp*cy + sr*sp*sy;
}

void Quaternion::ToEuler(float& oRoll, float& oPitch, float& oYaw)
{
	float a = 2.0f  *  (S * Vy - Vx * Vz);
	if (a < 1.0f)
	{
		if (-1.0f < a)
		{
			oRoll = atan2f(2.0f * (Vy * Vz + S * Vx), 1.0f - 2.0f * (Vx * Vx + Vy * Vy));
			oPitch = asinf(a);
			oYaw = atan2f(2.0f * (Vx * Vy + S * Vz), 1.0f - 2.0f * (Vy * Vy + Vz * Vz));
		} 
		else 
		{
			oRoll = -atan2f(2.0f * (Vx * Vy-S * Vz), 1.0f - 2.0f * (Vx * Vx + Vz * Vz));
			oPitch = Pi / -2.0f;
			oYaw = 0.0f;
		}
	} 
	else 
	{
		oRoll = atan2f(2.0f * (Vx * Vy - S * Vz), 1.0f - 2.0f * (Vx * Vx + Vz * Vz));
		oPitch = Pi / 2.0f;
		oYaw = 0.0f;
	}
}

Quaternion::Quaternion(Quaternion& Q1, Quaternion& Q2, float Ratio)
{
	float cosom = Q1.S * Q2.S + Q1.Vx * Q2.Vx + Q1.Vy * Q2.Vy + Q1.Vz * Q2.Vz;
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

	S = Q1.S * sclp + Q2.S * sclq;
	Vx = Q1.Vx * sclp + Q2.Vx* sclq;
	Vy = Q1.Vy * sclp + Q2.Vy * sclq;
	Vz = Q1.Vz * sclp + Q2.Vz * sclq;
}

Quaternion::Quaternion() {}

Quaternion::Quaternion( float _S, float _Vx, float _Vy, float _Vz ): S(_S), Vx(_Vx), Vy(_Vy), Vz(_Vz) {}

Quaternion& Quaternion::operator *= (Quaternion& Q)
{
	float bs = S*Q.S - Vx*Q.Vx - Vy*Q.Vy - Vz*Q.Vz;
	float bx = S*Q.Vx + Vx*Q.S + Vy*Q.Vz - Vz*Q.Vy;
	float by = S*Q.Vy + Vy*Q.S + Vz*Q.Vx - Vx*Q.Vz;
	float bz = S*Q.Vz + Vz*Q.S + Vx*Q.Vy - Vy*Q.Vx;

	S = bs;
	Vx = bx;
	Vy = by;
	Vz = bz;
	return *this;
}

Quaternion Quaternion::operator *(const Quaternion &Q)
{
	float bs = S*Q.S - Vx*Q.Vx - Vy*Q.Vy - Vz*Q.Vz;
	float bx = S*Q.Vx + Vx*Q.S + Vy*Q.Vz - Vz*Q.Vy;
	float by = S*Q.Vy + Vy*Q.S + Vz*Q.Vx - Vx*Q.Vz;
	float bz = S*Q.Vz + Vz*Q.S + Vx*Q.Vy - Vy*Q.Vx;
	return Quaternion(bs, bx, by, bz);	
}

Quaternion& Quaternion::operator = (const Quaternion& Q)
{
	S = Q.S;
	Vx = Q.Vx;
	Vy = Q.Vy;
	Vz = Q.Vz;
	return *this;
}

Quaternion Quaternion::Conjugate()
{
	return Quaternion(S, -Vx, -Vy, -Vz);
}