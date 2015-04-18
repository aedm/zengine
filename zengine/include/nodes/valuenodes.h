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
	return static_cast<ValueNode<T>*>(GetConnectedNode())->Get();
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



/// Nodes holding primitive values that can be directly set.
template<NodeType T>
class ValueNode: public Node
{
	typedef typename NodeTypes<T>::Type ValueType;

public:
	ValueNode();

	/// For cloning
	ValueNode(const ValueNode<T>& Original);

	/// Returns value of node. Reevaluates if necessary
	virtual const ValueType&	Get();

	/// Sets value of node. Operate() should call this, too.
	virtual void				Set(const ValueType& NewValue);

	/// Clone node
	virtual Node*				Clone() const override;

	/// This is a primitive type, value can be set directly
	virtual bool				CanSetValueDirectly();

protected:
	/// Output value of the node
	ValueType					Value;
};

typedef ValueNode<NodeType::FLOAT>		FloatNode;
typedef ValueNode<NodeType::VEC4>		Vec4Node;
typedef ValueNode<NodeType::MATRIX44>	Matrix4Node;

// TODO: kill this
typedef ValueNode<NodeType::TEXTURE>	TextureNode;


/// Explicit template type instantiations
template class ValueNode<NodeType::FLOAT>;
template class ValueNode<NodeType::VEC4>;
template class ValueNode<NodeType::MATRIX44>;


template<NodeType T>
ValueNode<T>::ValueNode()
	: Node(T, VariableNames[(UINT)T])
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
bool ValueNode<T>::CanSetValueDirectly()
{
	return true;
}


template<NodeType T>
const typename ValueNode<T>::ValueType& ValueNode<T>::Get()
{
	ASSERT(IsProperlyConnected);
	Evaluate();
	return Value;
}


template<NodeType T>
void ValueNode<T>::Set( const typename ValueNode<T>::ValueType& NewValue )
{
	Value = NewValue;
	SetDependantsDirty();
}


template<NodeType T>
Node* ValueNode<T>::Clone() const
{
	return new ValueNode<T>(*this);
}
