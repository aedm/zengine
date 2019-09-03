#include <include/shaders/valuetype.h>
#include <include/base/helpers.h>

UINT ValueTypeByteSize(ValueType type) {
  switch (type)
  {
  case ValueType::FLOAT:
    return 4;
  case ValueType::VEC2:
    return 8;
  case ValueType::VEC3:
    return 12;
  case ValueType::VEC4:
    return 16;
  case ValueType::MATRIX44:
    return 64;
  default:
    SHOULD_NOT_HAPPEN;
    return 0;
  }
}

ValueType VertexAttributeUsageToValueType(VertexAttributeUsage usage) {
  switch (usage)
  {
  case VertexAttributeUsage::POSITION:
    return ValueType::VEC3;
  case VertexAttributeUsage::TEXCOORD:
    return ValueType::VEC2;
  case VertexAttributeUsage::NORMAL:
    return ValueType::VEC3;
  case VertexAttributeUsage::TANGENT:
    return ValueType::VEC3;
  default:
    SHOULD_NOT_HAPPEN;
    return ValueType(-1);
  }
}

bool VertexFormat::HasAttribute(VertexAttributeUsage attrib) {
  return (mBinaryFormat & (1 << (UINT)attrib)) != 0;
}