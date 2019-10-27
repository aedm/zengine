#include <include/nodes/valuenodes.h>
#include <include/dom/nodetype.h>

template<> StaticValueNode<float>::StaticValueNode()
  : ValueNode<float>() {
  mValue = 0.0f;
}

template<> StaticValueNode<vec2>::StaticValueNode()
  : ValueNode<vec2>() {
  mValue = vec2(0, 0);
}

template<> StaticValueNode<vec3>::StaticValueNode()
  : ValueNode<vec3>()
{
  mValue = vec3(0, 0, 0);
}

template<> StaticValueNode<vec4>::StaticValueNode()
  : ValueNode<vec4>()
{
  mValue = vec4(0, 0, 0, 0);
}

template<> StaticValueNode<Matrix>::StaticValueNode()
  : ValueNode<Matrix>()
{
  mValue.LoadIdentity();
}

template<> StaticValueNode<std::string>::StaticValueNode()
  : ValueNode<std::string>()
{}

REGISTER_NODECLASS(FloatNode, "Float")
REGISTER_NODECLASS(vec2Node, "vec2")
REGISTER_NODECLASS(vec3Node, "vec3")
REGISTER_NODECLASS(vec4Node, "vec4")
REGISTER_NODECLASS(MatrixNode, "Matrix")


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
template class StaticValueNode<vec2>;
template class StaticValueNode<vec3>;
template class StaticValueNode<vec4>;
template class StaticValueNode<mat4x4>;
template class StaticValueNode<std::string>;