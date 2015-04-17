#pragma once

#include "../dom/node.h"
#include "../base/types.h"

/// Nodes holding primitive values that can be directly set.
template<NodeType T>
class ValueNode: public TypedNode<T>
{
public:
	ValueNode();

	/// For cloning
	ValueNode(const ValueNode<T>& Original);

	/// Returns value of node. Reevaluates if necessary
	virtual const ValueType&	GetValue() override;

	/// Sets value of node. Operate() should call this, too.
	virtual void				SetValue(const ValueType& NewValue);

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
	: TypedNode()
{
	Value = ValueType();
}


template<NodeType T>
ValueNode<T>::ValueNode( const ValueNode<T>& Original )
	: TypedNode(Original)
{
	Value = Original.Value;
}


template<NodeType T>
bool ValueNode<T>::CanSetValueDirectly()
{
	return true;
}


template<NodeType T>
const typename ValueNode<T>::ValueType& ValueNode<T>::GetValue()
{
	ASSERT(IsProperlyConnected);
	Evaluate();
	return Value;
}


template<NodeType T>
void ValueNode<T>::SetValue( const typename ValueNode<T>::ValueType& NewValue )
{
	Value = NewValue;
	SetDependantsDirty();
}


template<NodeType T>
Node* ValueNode<T>::Clone() const
{
	return new ValueNode<T>(*this);
}
