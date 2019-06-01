#include <include/shaders/shadervaluetype.h>
#include <include/base/helpers.h>

UINT ValueTypeByteSize(ShaderValueType type) {
  switch (type)
  {
  case ShaderValueType::FLOAT:
    return 4;
  case ShaderValueType::VEC2:
    return 8;
  case ShaderValueType::VEC3:
    return 12;
  case ShaderValueType::VEC4:
    return 16;
  case ShaderValueType::MATRIX44:
    return 64;
  default:
    SHOULD_NOT_HAPPEN;
    return 0;
  }
}
