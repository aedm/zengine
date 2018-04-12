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
#define ITEM(name, capitalizedName, type) make_shared<StaticValueNode<ValueType::name>>(),
shared_ptr<Node> StaticValueNodesList[] = {
  VALUETYPE_LIST
};


/// Slot factory
Slot* CreateValueSlot(ValueType type, Node* owner, SharedString name, 
  bool isMultiSlot, bool isPublic, bool isSerializable, float minimum, float maximum) 
{
  switch (type) {
#undef ITEM
#define ITEM(upperName, capitalizedName, type) \
    case ValueType::upperName: \
      return new capitalizedName##Slot(owner, name, \
        isMultiSlot, isPublic, isSerializable, minimum, maximum);
    VALUETYPE_LIST
  default:
    SHOULD_NOT_HAPPEN;
    return nullptr;
  }
}

