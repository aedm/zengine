#pragma once

#include "../dom/node.h"
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
template<> StaticValueNode<vec2>::StaticValueNode();
template<> StaticValueNode<vec3>::StaticValueNode();
template<> StaticValueNode<vec4>::StaticValueNode();
template<> StaticValueNode<mat4>::StaticValueNode();
template<> StaticValueNode<std::string>::StaticValueNode();


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
  ValueSlot(Node* owner, const std::string& name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true,
            float minimum = 0.0f, float maximum = 1.0f);

  const T& Get() const;

  /// Return minimum & maximum
  vec2 GetRange() const;

  /// Sets the value of the built-in Node.
  void SetDefaultValue(const T& value);

  /// Gets the default value
  const T& GetDefaultValue();

  ///// True if NodeValue a can be connected
  //virtual bool DoesAcceptValueNode(ValueType type) const override;

  /// Attaches slot to node. If the node parameter is nullptr, 
  /// the slot connects to the built-in node instead.
  bool Connect(const std::shared_ptr<Node>& target) override;

  /// Disconnects a node from this slot, and connects it
  /// to the built-in node.
  void Disconnect(const std::shared_ptr<Node>&) override;
  void DisconnectAll(bool notifyOwner) override;

  /// Returns true if slot is connected to its own default node
  bool IsDefaulted() override;

protected:
  /// Default value
  std::shared_ptr<StaticValueNode<T>> mDefault;
  float mMinimum;
  float mMaximum;
};


template<typename T>
ValueSlot<T>::ValueSlot(Node* owner, const std::string& name, bool isMultiSlot,
                        bool isPublic, bool isSerializable,
                        float minimum, float maximum)
  : TypedSlot<ValueNode<T>>(owner, name, isMultiSlot, isPublic, isSerializable)
  , mDefault(std::make_shared<StaticValueNode<T>>())
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
vec2 ValueSlot<T>::GetRange() const {
  return vec2(mMinimum, mMaximum);
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
bool ValueSlot<T>::Connect(const std::shared_ptr<Node>& target) {
  if (mNode == target || (target == nullptr && mNode == mDefault)) return true;
  if (target && !DoesAcceptNode(target)) {
    ERR("Slot and node type mismatch");
    DEBUGBREAK;
    return false;
  }
  if (mNode) mNode->DisconnectFromSlot(this);
  mNode = target ? target : mDefault;
  mNode->ConnectToSlot(this);
  mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
  return true;
}


template<typename T>
void ValueSlot<T>::Disconnect(const std::shared_ptr<Node>& target) {
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
typedef ValueSlot<vec2> Vec2Slot;
typedef ValueSlot<vec3> Vec3Slot;
typedef ValueSlot<vec4> Vec4Slot;
typedef ValueSlot<mat4> MatrixSlot;
typedef ValueSlot<std::string> StringSlot;

typedef StaticValueNode<float> FloatNode;
typedef StaticValueNode<vec2> Vec2Node;
typedef StaticValueNode<vec3> Vec3Node;
typedef StaticValueNode<vec4> Vec4Node;
typedef StaticValueNode<mat4> MatrixNode;
typedef StaticValueNode<std::string> StringNode;
