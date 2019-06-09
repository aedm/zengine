#include <include/nodes/vectornodes.h>

REGISTER_NODECLASS(FloatsToVec3Node, "Floats To Vec3");
REGISTER_NODECLASS(FloatsToVec4Node, "Floats To Vec4");
REGISTER_NODECLASS(FloatToFloatNode, "Float To Float");
REGISTER_NODECLASS(MaddNode, "MAdd");

FloatsToVec3Node::FloatsToVec3Node()
  : ValueNode<Vec3>()
  , mValue(0, 0, 0)
  , mX(this, "X")
  , mY(this, "Y")
  , mZ(this, "Z")
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
  : ValueNode<float>()
  , mX(this, "X")
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

FloatsToVec4Node::FloatsToVec4Node()
  : ValueNode<Vec4>()
  , mValue(0, 0, 0, 0)
  , mX(this, "X")
  , mY(this, "Y")
  , mZ(this, "Z")
  , mW(this, "W")
{}

const Vec4& FloatsToVec4Node::Get() {
  Update();
  return mValue;
}

void FloatsToVec4Node::HandleMessage(Message* message) {
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

void FloatsToVec4Node::Operate() {
  mValue = Vec4(mX.Get(), mY.Get(), mZ.Get(), mW.Get());
}

MaddNode::MaddNode()
  : ValueNode<float>()
  , mValue(1)
  , mA(this, "A")
  , mB(this, "B")
  , mC(this, "C")
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

