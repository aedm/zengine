#pragma once

#include "../dom/node.h"
#include "../dom/types.h"

/// Operators holding primitive values that can be directly set.
template<NodeTypeEnum T>
class ValueOperator: public TypedNode<T>
{
public:
	ValueOperator();

	/// For cloning
	ValueOperator(const ValueOperator<T>& Original);

	/// Returns value of operator. Reevaluates if necessary
	virtual const ValueType&	GetValue() override;

	/// Sets value of operator. Operate() should call this, too.
	virtual void				SetValue(const ValueType& NewValue);

	/// Clone operator
	virtual Node*				Clone() const override;

	/// This is a primitive type, value can be set directly
	virtual bool				CanSetValueDirectly();

protected:
	/// Output value of the operator
	ValueType					Value;
};

typedef ValueOperator<NODE_FLOAT>		FloatOperator;
typedef ValueOperator<NODE_VEC4>		Vec4Operator;
typedef ValueOperator<NODE_MATRIX44>	Matrix4Operator;
typedef ValueOperator<NODE_TEXURE>		TextureOperator;

/// Explicit template type instantiations
template class ValueOperator<NODE_FLOAT>;
template class ValueOperator<NODE_VEC4>;
template class ValueOperator<NODE_MATRIX44>;


template<NodeTypeEnum T>
ValueOperator<T>::ValueOperator()
	: TypedNode()
{
	Value = ValueType();
}


template<NodeTypeEnum T>
ValueOperator<T>::ValueOperator( const ValueOperator<T>& Original )
	: TypedNode(Original)
{
	Value = Original.Value;
}


template<NodeTypeEnum T>
bool ValueOperator<T>::CanSetValueDirectly()
{
	return true;
}


template<NodeTypeEnum T>
const typename ValueOperator<T>::ValueType& ValueOperator<T>::GetValue()
{
	ASSERT(IsProperlyConnected);
	Evaluate();
	return Value;
}


template<NodeTypeEnum T>
void ValueOperator<T>::SetValue( const typename ValueOperator<T>::ValueType& NewValue )
{
	Value = NewValue;
	SetDependantsDirty();
}


template<NodeTypeEnum T>
Node* ValueOperator<T>::Clone() const
{
	return new ValueOperator<T>(*this);
}
