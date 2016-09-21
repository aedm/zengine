#pragma once

#include "../dom/node.h"
#include "../base/types.h"
#include "../resources/texture.h"
#include <string>

/// Nodes holding primitive values.
template<NodeType T>
class ValueNode: public Node {
protected:
  typedef typename NodeTypes<T>::Type ValueType;

public:
  ValueNode();

  /// For cloning
  ValueNode(const ValueNode<T>& original);

  /// Returns value of node. Reevaluates if necessary
  virtual const ValueType& Get() = 0;
};


template<NodeType T>
ValueNode<T>::ValueNode()
  : Node(T) {}


template<NodeType T>
ValueNode<T>::ValueNode(const ValueNode<T>& original)
  : Node(original) {}


/// Simple static value nodes
template<NodeType T>
class StaticValueNode: public ValueNode <T> {
public:
  StaticValueNode();

  /// For cloning
  StaticValueNode(const StaticValueNode<T>& original);

  /// Returns value of node. Reevaluates if necessary
  virtual const ValueType& Get() override;

  /// Sets value of node.
  void Set(const ValueType& newValue);

protected:
  /// Output value of the node
  ValueType mValue;
};


template<> StaticValueNode<NodeType::VEC2>::StaticValueNode();
template<> StaticValueNode<NodeType::VEC3>::StaticValueNode();
template<> StaticValueNode<NodeType::VEC4>::StaticValueNode();

template<NodeType T>
StaticValueNode<T>::StaticValueNode()
  : ValueNode() {
  mValue = ValueType();
}


template<NodeType T>
StaticValueNode<T>::StaticValueNode(const StaticValueNode<T>& original)
  : ValueNode(original) {
  mValue = original.mValue;
}


template<NodeType T>
const typename StaticValueNode<T>::ValueType& StaticValueNode<T>::Get() {
  return mValue;
}


template<NodeType T>
void StaticValueNode<T>::Set(const typename ValueNode<T>::ValueType& newValue) {
  mValue = newValue;
  SendMsg(NodeMessage::VALUE_CHANGED);
  onSniffMessage(NodeMessage::VALUE_CHANGED, nullptr, nullptr);
}


/// Slots for value nodes. These slots have a StaticValueNode built in
/// that provides a default value. This way these nodes provide a value
/// even when there's no external node connected to them. Also, GetNode()
/// never returns nullptr.
template<NodeType T>
class ValueSlot: public Slot {
public:
  typedef typename NodeTypes<T>::Type ValueType;

  ValueSlot(Node* owner, SharedString name, bool isMultiSlot = false,
            bool isPublic = true, bool isSerializable = true);

  const ValueType& Get() const;

  /// Sets the value of the built-in Node.
  void SetDefaultValue(const ValueType& value);

  /// Gets the default value
  const ValueType& GetDefaultValue();

  /// Attaches slot to node. If the node parameter is nullptr, 
  /// the slot connects to the built-in node instead.
  virtual bool Connect(Node* node) override;

  /// Disconnects a node from this slot, and connects it
  /// to the built-in node.
  virtual void Disconnect(Node* node) override;
  virtual void DisconnectAll(bool notifyOwner) override;

  /// Returns true if slot is connected to its own default node
  virtual bool IsDefaulted() override;

protected:
  /// Default value
  StaticValueNode<T> mDefault;
};


template<NodeType T>
ValueSlot<T>::ValueSlot(Node* owner, SharedString name, bool isMultiSlot,
                        bool isPublic, bool isSerializable)
  : Slot(T, owner, name, isMultiSlot, isPublic, isSerializable) {
  Connect(&mDefault);
}


template<NodeType T>
const typename ValueSlot<T>::ValueType& ValueSlot<T>::Get() const {
  ASSERT(GetAbstractNode()->GetType() == T)
  return static_cast<ValueNode<T>*>(GetAbstractNode())->Get();
}


template<NodeType T>
void ValueSlot<T>::SetDefaultValue(const ValueType& value) {
  ASSERT(IsDefaulted());
  mDefault.Set(value);
}


template<NodeType T>
const typename ValueSlot<T>::ValueType& ValueSlot<T>::GetDefaultValue() {
  return mDefault.Get();
}


template<NodeType T>
bool ValueSlot<T>::Connect(Node* target) {
  if (mNode == target || (target == nullptr && mNode == &mDefault)) return true;
  if (target && !DoesAcceptType(target->GetType())) {
    DEBUGBREAK("Slot and operator type mismatch");
    return false;
  }
  if (mNode) mNode->DisconnectFromSlot(this);
  mNode = target ? target : &mDefault;
  mNode->ConnectToSlot(this);
  mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
  return true;
}


template<NodeType T>
void ValueSlot<T>::Disconnect(Node* target) {
  ASSERT(target == mNode);
  ASSERT(target != nullptr);
  mNode->DisconnectFromSlot(this);
  if (mNode == &mDefault) {
    /// Avoid infinite loop when called by the default node's destructor.
    mNode = nullptr;
    return;
  }
  mNode = &mDefault;
  mDefault.ConnectToSlot(this);
  mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
}


template<NodeType T>
void ValueSlot<T>::DisconnectAll(bool notifyOwner) {
  SHOULDNT_HAPPEN;
  if (mNode == &mDefault) return;
  mDefault.ConnectToSlot(this);
  if (notifyOwner) {
    mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
  }
}


template<NodeType T>
bool ValueSlot<T>::IsDefaulted() {
  return GetAbstractNode() == &mDefault;
}


/// Helper cast
template<NodeType T>
inline static ValueSlot<T>* ToValueSlot(Slot* slot) {
  return static_cast<ValueSlot<T>*>(slot);
}


/// Node and slot types
typedef ValueSlot<NodeType::FLOAT>			FloatSlot;
typedef ValueSlot<NodeType::VEC2>			  Vec2Slot;
typedef ValueSlot<NodeType::VEC3>			  Vec3Slot;
typedef ValueSlot<NodeType::VEC4>			  Vec4Slot;
typedef ValueSlot<NodeType::MATRIX44>		Matrix4Slot;

typedef StaticValueNode<NodeType::FLOAT>	  FloatNode;
typedef StaticValueNode<NodeType::VEC2>	    Vec2Node;
typedef StaticValueNode<NodeType::VEC3>	    Vec3Node;
typedef StaticValueNode<NodeType::VEC4>		  Vec4Node;
typedef StaticValueNode<NodeType::MATRIX44>	Matrix4Node;


///// Explicit template type instantiations
//template class StaticValueNode < NodeType::FLOAT >;
//template class StaticValueNode < NodeType::VEC4 >;
//template class StaticValueNode < NodeType::MATRIX44 >;
//template class StaticValueNode<NodeType::TEXTURE>;


/// Define texture type
template<> struct NodeTypes<NodeType::TEXTURE> { typedef Texture* Type; };
typedef StaticValueNode<NodeType::TEXTURE>	TextureNode;
typedef ValueSlot<NodeType::TEXTURE> TextureSlot;

/// Define text type
template<> struct NodeTypes<NodeType::STRING> { typedef std::string Type; };
typedef StaticValueNode<NodeType::STRING>	StringNode;
typedef ValueSlot<NodeType::STRING> StringSlot;


