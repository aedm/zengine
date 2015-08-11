#include <include/nodes/vectornodes.h>

REGISTER_NODECLASS(FloatsToVec3Node, "Floats To Vec3");

static SharedString XSlotName = make_shared<string>("X");
static SharedString YSlotName = make_shared<string>("Y");
static SharedString ZSlotName = make_shared<string>("Z");

FloatsToVec3Node::FloatsToVec3Node() 
  : ValueNode<NodeType::VEC3>()
  , mValue(0, 0, 0)
  , mX(this, XSlotName)
  , mY(this, YSlotName)
  , mZ(this, ZSlotName)
{  
}

const Vec3& FloatsToVec3Node::Get() {
  Update();
  return mValue;
}

FloatsToVec3Node::~FloatsToVec3Node() {}

void FloatsToVec3Node::Operate() {
  mValue = Vec3(mX.Get(), mY.Get(), mZ.Get());
}
