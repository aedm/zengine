#include <include/nodes/valuenodes.h>
#include <include/dom/nodetype.h>

template<> StaticValueNode<ValueType::VEC2>::StaticValueNode()
  : ValueNode<ValueType::VEC2>() {
  mValue = Vec2(0, 0);
}

template<> StaticValueNode<ValueType::VEC3>::StaticValueNode()
  : ValueNode<ValueType::VEC3>()
{
  mValue = Vec3(0, 0, 0);
}

template<> StaticValueNode<ValueType::VEC4>::StaticValueNode()
  : ValueNode<ValueType::VEC4>()
{
  mValue = Vec4(0, 0, 0, 0);
}

#undef ITEM
#define ITEM(name, type) static StaticValueNode<ValueType::name> name##NodeInstance;
VALUETYPE_LIST

#undef ITEM
#define ITEM(name, type) &name##NodeInstance,
Node* StaticValueNodesList[] = {
  VALUETYPE_LIST
};

