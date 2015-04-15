#pragma once

#include "types.h"
#include "../base/vectormath.h"
#include "../base/fastdelegate.h"
#include "../base/helpers.h"
#include <vector>
#include <memory>
#include <boost/foreach.hpp>

using namespace std;
using namespace fastdelegate;

class Node;
template<NodeTypeEnum T> class TypedOperator;

/// Nodes can have multiple input slots. Each slot's value is set by an another node's output.
class Slot
{
public:
	Slot(NodeTypeEnum Type, Node* Owner, SharedString Name);
	virtual ~Slot();

	/// The operator which this slot is a member of
	Node* const				Owner;

	/// Attach slot to ZenOperator. Slot value will be taken from operator value.
	/// If @Operator is NULL, slot will be detached. 
	/// Returns false if connection is not possible due to type mismatch.
	bool						Connect(Node* Op);

	/// Returns connected operator
	Node*					GetAttachedOperator() const;

	/// Type of object this slot accepts
	NodeTypeEnum					GetType() const;
	
	/// Returns the name of the slot
	SharedString				GetName();

protected:
	/// Slot is connected to this one
	Node*					AttachedOperator;

	/// Name of the slot. Can't be changed.
	SharedString				Name;

	/// Output type
	const NodeTypeEnum			Type;

	/// Biolerplate
	void						DisconnectFromOperator();
	void						FinalizeAttach();
};


/// Slot expecting a value operator.
template <NodeTypeEnum T>
class TypedSlot: public Slot
{
	typedef typename OpTypes<T>::Type ValueType;

public:
	TypedSlot(Node* Owner, SharedString Name);

	const ValueType&			Value();

	/// Returns connected typed operator
	TypedOperator<T>*			GetOperator() const;
};


/// An operation that takes its slot values as input and computes an output
class Node
{
	friend class Slot;

public:
	virtual ~Node();

	/// Parameters of this operator.
	vector<Slot*>				Slots;

	/// Name of the operator
	string						Name;
	
	/// Returns object type.
	NodeTypeEnum					GetType() const;			

	/// List of slots this operator's output is connected to
	const vector<Slot*>&		GetDependants() const;	

	/// Reruns Operate() if dirty (on dirty ancestors too)
	void						Evaluate();

	/// Clone operator
	virtual Node*			Clone() const;

	/// If true, this is a primitive type, value can be set directly
	virtual bool				CanSetValueDirectly();

protected:
	Node(const string& Name);

	/// For cloning
	Node(const Node& Original);

	/// True is all slots are properly connected
	bool						IsProperlyConnected;

	/// Main operation
	virtual void				Operate() {}

	/// Callback for when one of the slots connection has changed
	virtual void				OnSlotConnectionsChanged(Slot* S) {}

	/// Callback for when one of the slots value has changed
	virtual void				OnSlotValueChanged(Slot* DirtySlot);

	/// Notify slots about changed value
	void						SetDependantsDirty();
	
	/// Output type
	NodeTypeEnum					Type;

private:
	/// True if Operate() needs to be called
	bool						IsDirty;

	/// Slots this operator is connected to (as an input)
	vector<Slot*>				Dependants;

	/// Add or remove slot to/from notification list
	void						ConnectToSlot(Slot* S);			
	void						DisconnectFromSlot(Slot* S);	

	/// Check if all slots are properly connected to an operator
	void						CheckConnections();
};


/// Operator with an output value.
template<NodeTypeEnum T>
class TypedOperator: public Node
{
public:
	typedef typename OpTypes<T>::Type ValueType;

	TypedOperator();

	/// For cloning
	TypedOperator(const TypedOperator<T>& Original);

	/// Returns value of operator. Reevaluates if necessary
	virtual const ValueType&	GetValue() = NULL;
};


/// Operator and slot types
typedef TypedSlot<NODE_FLOAT>		FloatSlot;
typedef TypedSlot<NODE_VEC4>		Vec4Slot;
typedef TypedSlot<NODE_MATRIX44>	Matrix4Slot;
typedef TypedSlot<NODE_TEXURE>		TextureSlot;


/// Helper cast
template<NodeTypeEnum T> 
inline static TypedSlot<T>* ToValueSlot(Slot* slot) 
{ 
	return static_cast<TypedSlot<T>*>(slot); 
}


template <NodeTypeEnum T>
TypedSlot<T>::TypedSlot( Node* Owner, SharedString Name )
	: Slot(T, Owner, Name)
{}


template <NodeTypeEnum T>
const typename TypedSlot<T>::ValueType& TypedSlot<T>::Value()
{
	return static_cast<TypedOperator<T>*>(GetAttachedOperator())->GetValue();
}


template<NodeTypeEnum T>
TypedOperator<T>::TypedOperator()
	: Node(VariableNames[(UINT)T])
{
	Type = T;
}


template<NodeTypeEnum T>
TypedOperator<T>::TypedOperator( const TypedOperator<T>& Original ) 
	: Node(Original)
{}


