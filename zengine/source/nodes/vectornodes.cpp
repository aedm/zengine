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

void FloatsToVec3Node::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
    case MessageType::VALUE_CHANGED:
      mIsUpToDate = false;
      SendMsg(MessageType::VALUE_CHANGED);
      break;
    default:
      break;
  }
}

FloatsToVec3Node::~FloatsToVec3Node() {}

void FloatsToVec3Node::Operate() {
  mValue = Vec3(mX.Get(), mY.Get(), mZ.Get());
}
