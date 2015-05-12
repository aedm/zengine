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

/// Notifications that nodes send to each other when something changed.
/// An accompanying UINT data can also be set for each event type.
enum class NodeMessage {
	/// A slot was added or removed.
	SLOT_STRUCTURE_CHANGED,

	/// Direct slot connection changed.
	SLOT_CONNECTION_CHANGED,

	/// Some transitive connection changed
	TRANSITIVE_CONNECTION_CHANGED,

	/// The value of a connected node changed, reevaluation might be needed
	VALUE_CHANGED,

	/// Node looks changed, watchers need to redraw it.
	NEEDS_REDRAW,
};

class Node;

/// Nodes can have multiple input slots, which connects it to other slots.
class Slot
{
public:
	Slot(NodeType Type, Node* Owner, SharedString Name, bool IsMultiSlot = false,
		bool AutoAddToSlotList = true);
	virtual ~Slot();

	/// The operator which this slot is a member of
	Node* const				Owner;

	/// Attaches slot to node. 
	/// - for non-multislots, this overrides the current connection.
	/// - for multislots, the node will be added to the list of connected nodes. 
	/// Returns false if connection is not possible due to type mismatch.
	bool					Connect(Node* Nd);

	/// Disconnects a node from this slot. 
	void					Disconnect(Node* Nd);

	/// Removes the connected node from connected nodes list, 
	/// and reinserts it at the "TargetIndex" position. Only for multislots.
	void					ChangeNodeIndex(Node* Nd, UINT TargetIndex);

	/// Disconnects all nodes from this slot.
	void					DisconnectAll();

	/// Returns connected node (errorlog & nullptr if multislot)
	Node*					GetNode() const;

	/// Returns all connected nodes (only for multislot)
	const vector<Node*>&	GetMultiNodes() const;

	/// Type of object this slot accepts
	/// TODO: get rid of this
	NodeType				GetType() const;
	
	/// Returns the name of the slot
	SharedString			GetName();

	/// True if the slot can connect to multiple nodes
	const bool				IsMultiSlot;

	/// Return the Nth connected node from a multislot
	Node*					operator[] (UINT Index);

protected:
	/// The slot is connected to this node (nullptr if multislot)
	Node*					ConnectedNode;

	/// The slot is connected to these nodes (empty if not multislot)
	vector<Node*>			MultiNodes;

	/// Name of the slot. Can't be changed.
	SharedString			Name;

	/// Output type
	const NodeType			Type;
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

	/// Hook for watchers (UI only)
	Event<Slot*, NodeMessage, const void*> OnMessageReceived;

protected:
	Node(NodeType Type, const string& Name);

	/// For cloning
	Node(const Node& Original);

	/// True is all slots are properly connected
	bool						IsProperlyConnected;

	/// Main operation
	virtual void				Operate() {}

	/// Sends a message to dependants
	void						SendMessage(NodeMessage Message, const void* Payload = nullptr);

	/// Handle received messages
	virtual void				HandleMessage(Slot* S, NodeMessage Message, const void* Payload);

	/// Receives message through a slot
	void						ReceiveMessage(Slot* S, NodeMessage Message, const void* Payload = nullptr);

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



