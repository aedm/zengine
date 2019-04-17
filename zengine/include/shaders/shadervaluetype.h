#pragma once

#include "../base/vectormath.h"

// Possible types of shader values and variables
enum class ShaderValueType {
  FLOAT,
  VEC2,
  VEC3,
  VEC4,
  MATRIX44,
};

// Translation between shader types and C++ types
template<ShaderValueType T> struct ShaderValueTypes;
template<> struct ShaderValueTypes<ShaderValueType::FLOAT> { typedef float Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC2> { typedef Vec2 Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC3> { typedef Vec3 Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC4> { typedef Vec4 Type; };
template<> struct ShaderValueTypes<ShaderValueType::FLOAT> { typedef Matrix Type; };

