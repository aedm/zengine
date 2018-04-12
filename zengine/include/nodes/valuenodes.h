#pragma once

#include "../dom/node.h"
#include "../base/types.h"
#include "../resources/texture.h"
#include <string>

/// Nodes holding primitive values.
template<ValueType T>
class ValueNode: public Node {
protected:
  typedef typename ValueTypes<T>::Type VType;

public:
  ValueNode();

  /// Returns value of node. Reevaluates if necessary
  virtual const VType& Get() = 0;
};


template<ValueType T>
ValueNode<T>::ValueNode() {
  mValueType = T;
}


/// Simple static value nodes
template<ValueType T>
class StaticValueNode: public ValueNode <T> {
public:
  StaticValueNode();

  /// Returns value of node. Reevaluates if necessary
  virtual const VType& Get() override;

  /// Sets value of node.
  void Set(const VType& newValue, bool silent = false);

protected:
  /// Output value of the node
  VType mValue;
};


template<> StaticValueNode<ValueType::VEC2>::StaticValueNode();
template<> StaticValueNode<ValueType::VEC3>::StaticValueNode();
template<> StaticValueNode<ValueType::VEC4>::StaticValueNode();

template<ValueType T>
StaticValueNode<T>::StaticValueNode()
  : ValueNode() {
  mValue = VType();
}


template<ValueType T>
const typename StaticValueNode<T>::VType& StaticValueNode<T>::Get() {
  return mValue;
}


template<ValueType T>
void StaticValueNode<T>::Set(const typename ValueNode<T>::VType& newValue, bool silent) {
  if (mValue == newValue) return;
  mValue = newValue;
  if (!silent) {
    SendMsg(MessageType::VALUE_CHANGED);
  }
}


/// Slots for value nodes. These slots have a StaticValueNode built in
/// that provides a default value. This way these nodes provide a value
/// even when there's no external node connected to them. Also, GetNode()
/// never returns nullptr.
template<ValueType T>
class ValueSlot: public TypedSlot<ValueNode<T>> {
public:
  typedef typename ValueTypes<T>::Type VType;

  ValueSlot(Node* owner, SharedString name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true,
            float minimum = 0.0f, float maximum = 1.0f);

  const VType& Get() const;

  /// Return minimum & maximum
  Vec2 GetRange() const;

  /// Sets the value of the built-in Node.
  void SetDefaultValue(const VType& value, bool silent = false);

  /// Gets the default value
  const VType& GetDefaultValue();

  /// True if NodeValue a can be connected
  virtual bool DoesAcceptValueNode(ValueType type) const override;

  /// Attaches slot to node. If the node parameter is nullptr, 
  /// the slot connects to the built-in node instead.
  /// If the "silent" flag is set, no message will be sent.
  virtual bool Connect(const shared_ptr<Node>& node, bool silent = false) override;

  /// Disconnects a node from this slot, and connects it
  /// to the built-in node.
  virtual void Disconnect(const shared_ptr<Node>&, bool silent = false) override;
  virtual void DisconnectAll(bool notifyOwner) override;

  /// Returns true if slot is connected to its own default node
  virtual bool IsDefaulted() override;

protected:
  /// Default value
  shared_ptr<StaticValueNode<T>> mDefault;
  float mMinimum;
  float mMaximum;
};


template<ValueType T>
ValueSlot<T>::ValueSlot(Node* owner, SharedString name, bool isMultiSlot,
                        bool isPublic, bool isSerializable,
                        float minimum, float maximum)
  : TypedSlot<ValueNode<T>>(owner, name, isMultiSlot, isPublic, isSerializable)
  , mDefault(make_shared<StaticValueNode<T>>())
  , mMinimum(minimum)
  , mMaximum(maximum)
{
  Connect(mDefault, true);
}


template<ValueType T>
const typename ValueSlot<T>::VType& ValueSlot<T>::Get() const {
  return PointerCast<ValueNode<T>>(GetReferencedNode())->Get();
}


template<ValueType T>
Vec2 ValueSlot<T>::GetRange() const {
  return Vec2(mMinimum, mMaximum);
}


template<ValueType T>
void ValueSlot<T>::SetDefaultValue(const VType& value, bool silent) {
  mDefault->Set(value, silent);
}


template<ValueType T>
const typename ValueSlot<T>::VType& ValueSlot<T>::GetDefaultValue() {
  return mDefault->Get();
}


template<ValueType T>
bool ValueSlot<T>::DoesAcceptValueNode(ValueType type) const {
  return T == type;
}


template<ValueType T>
bool ValueSlot<T>::Connect(const shared_ptr<Node>& target, bool silent) {
  if (mNode == target || (target == nullptr && mNode == mDefault)) return true;
  if (target && !DoesAcceptNode(target)) {
    DEBUGBREAK("Slot and node type mismatch");
    return false;
  }
  if (mNode) mNode->DisconnectFromSlot(this);
  mNode = target ? target : mDefault;
  mNode->ConnectToSlot(this);
  if (!silent) {
    TheMessageQueue.Enqueue(
      target, GetOwner(), MessageType::SLOT_CONNECTION_CHANGED, this);
  }
  return true;
}


template<ValueType T>
void ValueSlot<T>::Disconnect(const shared_ptr<Node>& target, bool silent) {
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
  if (!silent) {
    TheMessageQueue.Enqueue(
      target, GetOwner(), MessageType::SLOT_CONNECTION_CHANGED, this);
  }
}


template<ValueType T>
void ValueSlot<T>::DisconnectAll(bool notifyOwner) {
  SHOULD_NOT_HAPPEN;
  if (mNode == mDefault) return;
  mDefault->ConnectToSlot(this);
  if (notifyOwner) {
    TheMessageQueue.Enqueue(
      nullptr, GetOwner(), MessageType::SLOT_CONNECTION_CHANGED, this);
  }
}


template<ValueType T>
bool ValueSlot<T>::IsDefaulted() {
  return GetReferencedNode() == mDefault;
}

/// Node and slot types
#undef ITEM
#define ITEM(name, capitalizedName, type) \
typedef ValueSlot<ValueType::name> capitalizedName##Slot; \
typedef StaticValueNode<ValueType::name> capitalizedName##Node;
VALUETYPE_LIST

Slot* CreateValueSlot(ValueType type, Node* owner, 
  SharedString name, bool isMultiSlot = false,
  bool isPublic = true, bool isSerializable = true,
  float minimum = 0.0f, float maximum = 1.0f);

/// An instance of each static value nodes
extern shared_ptr<Node> StaticValueNodesList[];
