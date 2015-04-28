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
};

template<NodeType T>
ValueSlot<T>::ValueSlot(Node* Owner, SharedString Name)
	: Slot(T, Owner, Name)
{}

template<NodeType T>
const typename ValueSlot<T>::ValueType& ValueSlot<T>::Get() const
{
	return static_cast<ValueNode<T>*>(GetNode())->Get();
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
	virtual const ValueType&	Get();

	/// Clone node
	virtual Node*				Clone() const override;

protected:
	/// Output value of the node
	ValueType					Value;
};


template<NodeType T>
ValueNode<T>::ValueNode(const string& Name)
	: Node(T, Name)
{
	Value = ValueType();
}


template<NodeType T>
ValueNode<T>::ValueNode( const ValueNode<T>& Original )
	: Node(Original)
{
	Value = Original.Value;
}


template<NodeType T>
const typename ValueNode<T>::ValueType& ValueNode<T>::Get()
{
	ASSERT(IsProperlyConnected);
	Evaluate();
	return Value;
}


template<NodeType T>
Node* ValueNode<T>::Clone() const
{
	return new ValueNode<T>(*this);
}



/// Simple static value nodes
template<NodeType T>
class StaticValueNode : public ValueNode < T >
{
public:
	StaticValueNode();

	/// Sets value of node.
	void						Set(const ValueType& NewValue);
};


template<NodeType T>
StaticValueNode<T>::StaticValueNode()
	: ValueNode(VariableNames[(UINT)T])
{}


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
