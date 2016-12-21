#include <include/nodes/valuenodes.h>
#include <include/dom/nodetype.h>

template<> StaticValueNode<NodeType::VEC2>::StaticValueNode()
  : ValueNode<NodeType::VEC2>() {
  mValue = Vec2(0, 0);
}

template<> StaticValueNode<NodeType::VEC3>::StaticValueNode()
  : ValueNode<NodeType::VEC3>() 
{
  mValue = Vec3(0, 0, 0);
}

template<> StaticValueNode<NodeType::VEC4>::StaticValueNode() 
  : ValueNode<NodeType::VEC4>() 
{
  mValue = Vec4(0, 0, 0, 0);
}

static FloatNode a;