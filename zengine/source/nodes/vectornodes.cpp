#include <include/nodes/vectornodes.h>

REGISTER_NODECLASS(FloatsToVec3Node, "Floats To Vec3");
REGISTER_NODECLASS(FloatToFloatNode, "Float To Float");
REGISTER_NODECLASS(MaddNode, "MAdd");

static SharedString XSlotName = make_shared<string>("X");
static SharedString YSlotName = make_shared<string>("Y");
static SharedString ZSlotName = make_shared<string>("Z");

FloatsToVec3Node::FloatsToVec3Node()
  : ValueNode<ValueType::VEC3>()
  , mValue(0, 0, 0)
  , mX(this, XSlotName)
  , mY(this, YSlotName)
  , mZ(this, ZSlotName)
{}

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

void FloatsToVec3Node::Operate() {
  mValue = Vec3(mX.Get(), mY.Get(), mZ.Get());
}

FloatToFloatNode::FloatToFloatNode()
  : ValueNode<ValueType::FLOAT>()
  , mX(this, XSlotName)
  , mValue(0)
{}

const float& FloatToFloatNode::Get() {
  Update();
  return mValue;
}

void FloatToFloatNode::HandleMessage(Message* message) {
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

void FloatToFloatNode::Operate() {
  mValue = mX.Get();
}

static SharedString ASlotName = make_shared<string>("A");
static SharedString BSlotName = make_shared<string>("B");
static SharedString CSlotName = make_shared<string>("C");

MaddNode::MaddNode()
  : ValueNode<ValueType::FLOAT>()
  , mValue(1)
  , mA(this, ASlotName)
  , mB(this, BSlotName)
  , mC(this, CSlotName)
{
  mA.SetDefaultValue(1);
  mB.SetDefaultValue(1);
  mC.SetDefaultValue(0);
}

const float& MaddNode::Get() {
  Update();
  return mValue;
}

void MaddNode::HandleMessage(Message* message) {
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

void MaddNode::Operate() {
  mValue = mA.Get() * mB.Get() + mC.Get();
}