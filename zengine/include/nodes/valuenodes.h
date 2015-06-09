#pragma once

#include "../dom/node.h"
#include "../base/types.h"

/// Nodes holding primitive values.
template<NodeType T>
class ValueNode: public Node
{
protected:
	typedef typename NodeTypes<T>::Type ValueType;

public:
	ValueNode(const string& Name);

	/// For cloning
	ValueNode(const ValueNode<T>& Original);

	/// Returns value of node. Reevaluates if necessary
	virtual const ValueType&	Get() = 0;
};


template<NodeType T>
ValueNode<T>::ValueNode(const string& Name)
	: Node(T, Name)
{}


template<NodeType T>
ValueNode<T>::ValueNode( const ValueNode<T>& Original )
	: Node(Original)
{}


/// Simple static value nodes
template<NodeType T>
class StaticValueNode : public ValueNode < T >
{
public:
	StaticValueNode();

	/// For cloning
	StaticValueNode(const StaticValueNode<T>& Original);

	/// Returns value of node. Reevaluates if necessary
	virtual const ValueType&	Get() override;

	/// Sets value of node.
	void						Set(const ValueType& NewValue);

	/// Clone node
	virtual Node*				Clone() const override;

protected:
	/// Output value of the node
	ValueType					Value;
};


template<NodeType T>
StaticValueNode<T>::StaticValueNode()
	: ValueNode(VariableNames[(UINT)T])
{
	Value = ValueType();
}


template<NodeType T>
StaticValueNode<T>::StaticValueNode(const StaticValueNode<T>& Original)
	: ValueNode(Original)
{
	Value = Original.Value;
}


template<NodeType T>
Node* StaticValueNode<T>::Clone() const
{
	return new StaticValueNode<T>(*this);
}


template<NodeType T>
const typename StaticValueNode<T>::ValueType& StaticValueNode<T>::Get()
{
	return Value;
}


template<NodeType T>
void StaticValueNode<T>::Set(const typename ValueNode<T>::ValueType& NewValue)
{
	Value = NewValue;
	SendMessage(NodeMessage::VALUE_CHANGED);
}


/// Slots for value nodes. These slots have a StaticValueNode built in
/// that provides a default value. This way these nodes provide a value
/// even when there's no external node connected to them. Also, GetNode()
/// never returns nullptr.
template<NodeType T>
class ValueSlot : public Slot
{
public:
	typedef typename NodeTypes<T>::Type ValueType;

	ValueSlot(Node* Owner, SharedString Name);

	const ValueType&			Get() const;

	/// Sets the value of the built-in Node.
	void						SetDefaultValue(const ValueType& Value);

	/// Attaches slot to node. If the node parameter is nullptr, 
	/// the slot connects to the built-in node instead.
	virtual bool				Connect(Node* Nd) override;

	/// Disconnects a node from this slot, and connects it
	/// to the built-in node.
	virtual void				Disconnect(Node* Nd) override;
	virtual void				DisconnectAll(bool NotifyOwner) override;
	
	/// Returns true if slot is connected to its own default node
	bool						IsDefaulted();

protected:
	/// Default value
	StaticValueNode<T>			Default;
};


template<NodeType T>
ValueSlot<T>::ValueSlot(Node* Owner, SharedString Name)
	: Slot(T, Owner, Name)
{
	Default.Set(ValueType());
	Connect(&Default);
}


template<NodeType T>
const typename ValueSlot<T>::ValueType& ValueSlot<T>::Get() const
{
	return static_cast<ValueNode<T>*>(GetNode())->Get();
}


template<NodeType T>
void ValueSlot<T>::SetDefaultValue(const ValueType& Value)
{
	ASSERT(IsDefaulted());
	Default.Set(Value);
	Owner->HandleMessage(this, NodeMessage::VALUE_CHANGED);
}


template<NodeType T>
bool ValueSlot<T>::Connect(Node* Nd)
{
	if (ConnectedNode == Nd || (Nd == nullptr && ConnectedNode == &Default)) return true;
	if (Nd && Nd->GetType() != Type)
	{
		/// TODO: use ASSERT only
		ERR("Slot and operator type mismatch");
		ASSERT(false);
		return false;
	}
	if (ConnectedNode) ConnectedNode->DisconnectFromSlot(this);
	ConnectedNode = Nd ? Nd : &Default;
	ConnectedNode->ConnectToSlot(this);
	Owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	return true;
}


template<NodeType T>
void ValueSlot<T>::Disconnect(Node* Nd)
{
	ASSERT(Nd == ConnectedNode);
	ASSERT(Nd != nullptr);
	ConnectedNode->DisconnectFromSlot(this);
	ConnectedNode = nullptr;
	if (ConnectedNode == &Default)
	{
		/// Avoid infinite loop when called by the default node's destructor.
		return;
	}
	Default.ConnectToSlot(this);
	Owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
}


template<NodeType T>
void ValueSlot<T>::DisconnectAll(bool NotifyOwner)
{
	if (ConnectedNode == &Default) return;
	Default.ConnectToSlot(this);
	if (NotifyOwner) {
		Owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
}


template<NodeType T>
bool ValueSlot<T>::IsDefaulted()
{
	return GetNode() == &Default;
}


/// Helper cast
template<NodeType T>
inline static ValueSlot<T>* ToValueSlot(Slot* slot)
{
	return static_cast<ValueSlot<T>*>(slot);
}


/// Node and slot types
typedef ValueSlot<NodeType::FLOAT>			FloatSlot;
typedef ValueSlot<NodeType::VEC4>			Vec4Slot;
typedef ValueSlot<NodeType::MATRIX44>		Matrix4Slot;
typedef ValueSlot<NodeType::TEXTURE>		TextureSlot;

typedef StaticValueNode<NodeType::FLOAT>	FloatNode;
typedef StaticValueNode<NodeType::VEC4>		Vec4Node;
typedef StaticValueNode<NodeType::MATRIX44>	Matrix4Node;
typedef StaticValueNode<NodeType::TEXTURE>	TextureNode;


///// Explicit template type instantiations
//template class StaticValueNode < NodeType::FLOAT >;
//template class StaticValueNode < NodeType::VEC4 >;
//template class StaticValueNode < NodeType::MATRIX44 >;
