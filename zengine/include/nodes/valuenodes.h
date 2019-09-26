#pragma once

#include "../dom/node.h"
#include "../base/vectormath.h"
#include <string>

/// Nodes holding primitive values.
template<typename T>
class ValueNode: public Node {
public:
  /// Returns value of node. Reevaluates if necessary
  virtual const T& Get() = 0;
};


/// Simple static value nodes
template<typename T>
class StaticValueNode: public ValueNode<T> {
public:
  StaticValueNode();

  /// Returns value of node. Reevaluates if necessary
  const T& Get() override;

  /// Sets value of node.
  void Set(const T& newValue);

protected:
  /// Output value of the node
  T mValue;
};


template<> StaticValueNode<float>::StaticValueNode();
template<> StaticValueNode<Vec2>::StaticValueNode();
template<> StaticValueNode<Vec3>::StaticValueNode();
template<> StaticValueNode<Vec4>::StaticValueNode();
template<> StaticValueNode<Matrix>::StaticValueNode();
template<> StaticValueNode<string>::StaticValueNode();


template<typename T>
const T& StaticValueNode<T>::Get() {
  return mValue;
}





/// Slots for value nodes. These slots have a StaticValueNode built in
/// that provides a default value. This way these nodes provide a value
/// even when there's no external node connected to them. Also, GetNode()
/// never returns nullptr.
template<typename T>
class ValueSlot: public TypedSlot<ValueNode<T>> {
public:
  ValueSlot(Node* owner, const string& name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true,
            float minimum = 0.0f, float maximum = 1.0f);

  const T& Get() const;

  /// Return minimum & maximum
  Vec2 GetRange() const;

  /// Sets the value of the built-in Node.
  void SetDefaultValue(const T& value);

  /// Gets the default value
  const T& GetDefaultValue();

  ///// True if NodeValue a can be connected
  //virtual bool DoesAcceptValueNode(ValueType type) const override;

  /// Attaches slot to node. If the node parameter is nullptr, 
  /// the slot connects to the built-in node instead.
  bool Connect(const shared_ptr<Node>& target) override;

  /// Disconnects a node from this slot, and connects it
  /// to the built-in node.
  void Disconnect(const shared_ptr<Node>&) override;
  void DisconnectAll(bool notifyOwner) override;

  /// Returns true if slot is connected to its own default node
  bool IsDefaulted() override;

protected:
  /// Default value
  shared_ptr<StaticValueNode<T>> mDefault;
  float mMinimum;
  float mMaximum;
};


template<typename T>
ValueSlot<T>::ValueSlot(Node* owner, const string& name, bool isMultiSlot,
                        bool isPublic, bool isSerializable,
                        float minimum, float maximum)
  : TypedSlot<ValueNode<T>>(owner, name, isMultiSlot, isPublic, isSerializable)
  , mDefault(make_shared<StaticValueNode<T>>())
  , mMinimum(minimum)
  , mMaximum(maximum)
{
  Connect(mDefault);
}


template<typename T>
const T& ValueSlot<T>::Get() const {
  return PointerCast<ValueNode<T>>(this->GetReferencedNode())->Get();
}


template<typename T>
Vec2 ValueSlot<T>::GetRange() const {
  return Vec2(mMinimum, mMaximum);
}


template<typename T>
void ValueSlot<T>::SetDefaultValue(const T& value) {
  mDefault->Set(value);
}


template<typename T>
const T& ValueSlot<T>::GetDefaultValue() {
  return mDefault->Get();
}


template<typename T>
bool ValueSlot<T>::Connect(const shared_ptr<Node>& target) {
  if (mNode == target || (target == nullptr && mNode == mDefault)) return true;
  if (target && !DoesAcceptNode(target)) {
    DEBUGBREAK("Slot and node type mismatch");
    return false;
  }
  if (mNode) mNode->DisconnectFromSlot(this);
  mNode = target ? target : mDefault;
  mNode->ConnectToSlot(this);
  mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
  return true;
}


template<typename T>
void ValueSlot<T>::Disconnect(const shared_ptr<Node>& target) {
  ASSERT(target == mNode);
  ASSERT(target != nullptr);
  mNode->DisconnectFromSlot(this);
  if (mNode == mDefault) {
    /// Avoid infinite loop when called by the default node's destructor.
    mNode = nullptr;
    return;
  }
  mNode = mDefault;
  mDefault->ConnectToSlot(this);
  mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
}


template<typename T>
void ValueSlot<T>::DisconnectAll(bool notifyOwner) {
  if (mNode == mDefault) return;
  mNode->DisconnectFromSlot(this);
  mDefault->ConnectToSlot(this);
  mNode = mDefault;
  if (notifyOwner) {
    mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this);
  }
}


template<typename T>
bool ValueSlot<T>::IsDefaulted() {
  return GetReferencedNode() == mDefault;
}


/// Node and slot types
typedef ValueSlot<float> FloatSlot;
typedef ValueSlot<Vec2> Vec2Slot;
typedef ValueSlot<Vec3> Vec3Slot;
typedef ValueSlot<Vec4> Vec4Slot;
typedef ValueSlot<Matrix> MatrixSlot;
typedef ValueSlot<string> StringSlot;

typedef StaticValueNode<float> FloatNode;
typedef StaticValueNode<Vec2> Vec2Node;
typedef StaticValueNode<Vec3> Vec3Node;
typedef StaticValueNode<Vec4> Vec4Node;
typedef StaticValueNode<Matrix> MatrixNode;
typedef StaticValueNode<string> StringNode;
