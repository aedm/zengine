#pragma once

#include "../base/types.h"
#include "../base/vectormath.h"
#include "../base/fastdelegate.h"
#include "../base/helpers.h"
#include <vector>
#include <memory>
#include <boost/foreach.hpp>

using namespace std;
using namespace fastdelegate;

class Node;

/// Nodes can have multiple input slots. Each slot's value is set by an another node's output.
class Slot
{
public:
	Slot(NodeType Type, Node* Owner, SharedString Name);
	virtual ~Slot();

	/// The operator which this slot is a member of
	Node* const				Owner;

	/// Attach slot to node. Slot value will be taken from the node.
	/// If node parameter is nullptr, slot will be detached. 
	/// Returns false if connection is not possible due to type mismatch.
	bool					Connect(Node* Nd);

	/// Returns connected node
	Node*					GetConnectedNode() const;

	/// Type of object this slot accepts
	/// TODO: get rid of this
	NodeType				GetType() const;
	
	/// Returns the name of the slot
	SharedString			GetName();

protected:
	/// Slot is connected to this one
	Node*					ConnectedNode;

	/// Name of the slot. Can't be changed.
	SharedString			Name;

	/// Output type
	const NodeType			Type;

	/// Biolerplate
	void					DisconnectFromNode();
	void					FinalizeAttach();
};


/// An operation that takes its slot values as input and computes an output
class Node
{
	friend class Slot;

public:
	virtual ~Node();

	/// Parameters of this node.
	vector<Slot*>				Slots;

	/// Name of the node
	string						Name;
	
	/// Returns object type.
	NodeType					GetType() const;			

	/// List of slots this node's output is connected to
	const vector<Slot*>&		GetDependants() const;	

	/// Reruns Operate() if dirty (on dirty ancestors too)
	void						Evaluate();

	/// Clone node
	virtual Node*				Clone() const;

	/// If true, this is a primitive type, value can be set directly
	/// TODO: kill this
	virtual bool				CanSetValueDirectly();

protected:
	Node(NodeType Type, const string& Name);

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
	NodeType					Type;

private:
	/// True if Operate() needs to be called
	bool						IsDirty;

	/// Slots that this node is connected to (as an input)
	vector<Slot*>				Dependants;

	/// Add or remove slot to/from notification list
	void						ConnectToSlot(Slot* S);			
	void						DisconnectFromSlot(Slot* S);	

	/// Check if all slots are properly connected to an operator
	void						CheckConnections();
};



