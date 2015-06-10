#pragma once

#include "defines.h"

class Quaternion;

class Vec2
{
public:
	Vec2();
	Vec2(float x, float y);

	Vec2 operator + (const Vec2& op) const;
	Vec2 operator - (const Vec2& op) const;
	Vec2 operator / (const Vec2& op) const;
	Vec2 operator * (const Vec2& op) const;
	Vec2 operator * (float F) const;
	Vec2 operator / (float F) const;
	Vec2& operator *= (float F);

	float	Length() const;

	float	x, y;
};

class Vec3
{
public:
	Vec3();
	Vec3(float x, float y, float z);

	float x, y, z;

	Vec3 operator * (float f) const;
	Vec3 operator - (const Vec3& op) const;
	Vec3 operator + (const Vec3& op) const;
	Vec3 operator - () const;
	Vec3 operator / (float f) const;

	Vec3& operator /= (float f);
	Vec3& operator *= (float f);
	Vec3& operator += (const Vec3& op);

	Vec3 Cross(const Vec3& op) const;
	float Dot(const Vec3& op) const;
	float Length() const;
	float LengthSquared() const;
	Vec3 Normal() const;
  
	Vec3& Normalize();
};


class Vec4
{
public:
	Vec4();
	Vec4(float x, float y, float z, float w);
	Vec4(Vec3 v, float w);

	float x, y, z, w;

	float Dot(const Vec4& v) const;
	Vec4 operator - (const Vec4& v) const;
	bool operator == (const Vec4& v) const;

	Vec3 XYZ();
};

class Matrix
{
public:
	Matrix();

	float& operator() (UINT y, UINT x);
	Matrix& operator *= (const Matrix& m);
	Matrix operator * (const Matrix& m) const;
	Vec4 operator * (const Vec4& v) const;

	void LoadIdentity();

	static Matrix	Translate(const Vec3& translateVector);
	static Matrix	Rotate(float angle, const Vec3& axis);
	static Matrix	Rotate(const Quaternion& q);
	static Matrix	Scale(const Vec3& scaleVector);

	/// Ortho matrix where (0,0) is the top left
	static Matrix	Ortho(float x1, float y1, float x2, float y2);

	float			m[16];
};


class Quaternion
{
public:	
	Quaternion();
  Quaternion(float s, float vx, float vy, float vz);

  /// Spherical interpolation constructor
	Quaternion(Quaternion& q1, Quaternion& q2, float ratio);
	
	float s, vx, vy, vz;							

	void FromEuler(float roll, float pitch, float yaw);
	void ToEuler(float& oRoll, float& oPitch, float& oYaw);

	Quaternion Conjugate();

	Quaternion& operator *= (Quaternion& q);				
	Quaternion operator * (const Quaternion& q);				
	Quaternion& operator = (const Quaternion& q);					
};
