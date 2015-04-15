#pragma once

#include "defines.h"

class Quaternion;

class Vec2
{
public:
	Vec2();
	Vec2(float X, float Y);

	Vec2			operator + (const Vec2& op) const;
	Vec2			operator - (const Vec2& op) const;
	Vec2			operator / (const Vec2& op) const;
	Vec2			operator * (const Vec2& op) const;
	Vec2			operator * (float F) const;
	Vec2			operator / (float F) const;
	Vec2&			operator *= (float F);

	float			Length() const;

	float			X, Y;
};

class Vec3
{
public:
	Vec3();
	Vec3(float X, float Y, float Z);

	float			X, Y, Z;

	Vec3			operator * (float F) const;
	Vec3			operator - (const Vec3& op) const;
	Vec3			operator + (const Vec3& op) const;
	Vec3			operator - () const;
	Vec3			operator / (float F) const;

	Vec3&			operator /= (float F);
	Vec3&			operator *= (float F);
	Vec3&			operator += (const Vec3& op);

	Vec3			Cross(const Vec3& op) const;
	float			Dot(const Vec3& op) const;
	float			Length() const;
	float			LengthSquared() const;
	Vec3			Normal() const;


	Vec3&			Normalize();
};


class Vec4
{
public:
	Vec4();
	Vec4(float X, float Y, float Z, float W);
	Vec4(Vec3 V, float W);

	float			X, Y, Z, W;

	float			Dot(const Vec4& V) const;
	Vec4			operator - (const Vec4& V) const;
	bool			operator == (const Vec4& V) const;

	Vec3			XYZ();
};

class Matrix
{
public:
	Matrix();

	float&			operator() (UINT Y, UINT X);
	Matrix&			operator *= (const Matrix& M);
	Matrix			operator * (const Matrix& M) const;
	Vec4			operator * (const Vec4& V) const;

	void			LoadIdentity();

	static Matrix	Translate(const Vec3& TranslateVector);
	static Matrix	Rotate(float Angle, const Vec3& Axis);
	static Matrix	Rotate(const Quaternion& Q);
	static Matrix	Scale(const Vec3& ScaleVector);

	/// Ortho matrix where (0,0) is the top left
	static Matrix	Ortho(float x1, float y1, float x2, float y2);

	float			m[16];
};


class Quaternion
{
public:	
	Quaternion();
	Quaternion(Quaternion& Q1, Quaternion& Q2, float Ratio);			/// Slerp
	Quaternion(float S, float Vx, float Vy, float Vz);

	float			S, Vx, Vy, Vz;							

	void			FromEuler(float Roll, float Pitch, float Yaw);
	void			ToEuler(float& oRoll, float& oPitch, float& oYaw);

	Quaternion		Conjugate();

	Quaternion&		operator *= (Quaternion& Q);				
	Quaternion		operator * (const Quaternion& Q);				
	Quaternion&		operator = (const Quaternion& Q);					
};
