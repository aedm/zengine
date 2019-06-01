#include <include/nodes/valuenodes.h>
#include <include/dom/nodetype.h>

template<> StaticValueNode<float>::StaticValueNode()
  : ValueNode<float>() {
  mValue = 0.0f;
}

template<> StaticValueNode<Vec2>::StaticValueNode()
  : ValueNode<Vec2>() {
  mValue = Vec2(0, 0);
}

template<> StaticValueNode<Vec3>::StaticValueNode()
  : ValueNode<Vec3>()
{
  mValue = Vec3(0, 0, 0);
}

template<> StaticValueNode<Vec4>::StaticValueNode()
  : ValueNode<Vec4>()
{
  mValue = Vec4(0, 0, 0, 0);
}

template<> StaticValueNode<Matrix>::StaticValueNode()
  : ValueNode<Matrix>()
{
  mValue.LoadIdentity();
}

template<> StaticValueNode<string>::StaticValueNode()
  : ValueNode<string>()
{}

REGISTER_NODECLASS(FloatNode, "Float")
REGISTER_NODECLASS(Vec2Node, "Vec2")
REGISTER_NODECLASS(Vec3Node, "Vec3")
REGISTER_NODECLASS(Vec4Node, "Vec4")
REGISTER_NODECLASS(MatrixNode, "Matrix")


//#undef ITEM
//#define ITEM(name, capitalizedName, type) make_shared<StaticValueNode<ValueType::name>>(),
//shared_ptr<Node> StaticValueNodesList[] = {
//  VALUETYPE_LIST
//};


///// Slot factory
//Slot* CreateValueSlot(ValueType type, Node* owner, SharedString name, 
//  bool isMultiSlot, bool isPublic, bool isSerializable, float minimum, float maximum) 
//{
//  switch (type) {
//#undef ITEM
//#define ITEM(upperName, capitalizedName, type) \
//    case ValueType::upperName: \
//      return new capitalizedName##Slot(owner, name, \
//        isMultiSlot, isPublic, isSerializable, minimum, maximum);
//    VALUETYPE_LIST
//  default:
//    SHOULD_NOT_HAPPEN;
//    return nullptr;
//  }
//}

template<>
void StaticValueNode<Matrix>::Set(const Matrix& newValue) {
  mValue = newValue;
  SendMsg(MessageType::VALUE_CHANGED);
}

template<typename T>
void StaticValueNode<T>::Set(const T& newValue) {
  if (mValue == newValue) return;
  mValue = newValue;
  SendMsg(MessageType::VALUE_CHANGED);
}

template class StaticValueNode<float>;
template class StaticValueNode<Vec2>;
template class StaticValueNode<Vec3>;
template class StaticValueNode<Vec4>;
template class StaticValueNode<Matrix>;
template class StaticValueNode<string>;