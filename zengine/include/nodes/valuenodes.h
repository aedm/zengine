#pragma once

#include "../dom/node.h"
#include "../base/types.h"

/// Slots for value nodes
template<NodeType T>
class ValueSlot : public Slot
{
public:
	typedef typename NodeTypes<T>::Type ValueType;

	ValueSlot(Node* Owner, SharedString Name);

	const ValueType&			Get() const;

	void						SetDefaultValue(const ValueType& Value);

protected:
	/// Default value
	ValueType					DefaultValue;
};

template<NodeType T>
ValueSlot<T>::ValueSlot(Node* Owner, SharedString Name)
	: Slot(T, Owner, Name)
	, DefaultValue(ValueType())
{}

template<NodeType T>
const typename ValueSlot<T>::ValueType& ValueSlot<T>::Get() const
{
	Node* node = GetNode();
	return node ? static_cast<ValueNode<T>*>(node)->Get() : DefaultValue;
}

template<NodeType T>
void ValueSlot<T>::SetDefaultValue(const ValueType& Value)
{
	DefaultValue = Value;
	if (GetNode() != nullptr) Owner->HandleMessage(this, NodeMessage::VALUE_CHANGED);
}


/// Helper cast
template<NodeType T>
inline static ValueSlot<T>* ToValueSlot(Slot* slot)
{
	return static_cast<ValueSlot<T>*>(slot);
}

/// Node and slot types
typedef ValueSlot<NodeType::FLOAT>		FloatSlot;
typedef ValueSlot<NodeType::VEC4>		Vec4Slot;
typedef ValueSlot<NodeType::MATRIX44>	Matrix4Slot;
typedef ValueSlot<NodeType::TEXTURE>	TextureSlot;



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


typedef StaticValueNode<NodeType::FLOAT>	FloatNode;
typedef StaticValueNode<NodeType::VEC4>		Vec4Node;
typedef StaticValueNode<NodeType::MATRIX44>	Matrix4Node;
typedef StaticValueNode<NodeType::TEXTURE>	TextureNode;


///// Explicit template type instantiations
//template class StaticValueNode < NodeType::FLOAT >;
//template class StaticValueNode < NodeType::VEC4 >;
//template class StaticValueNode < NodeType::MATRIX44 >;
