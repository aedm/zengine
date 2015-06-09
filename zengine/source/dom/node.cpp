#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeType _Type, Node* _Owner, SharedString _Name, bool _IsMultiSlot, 
	bool AutoAddToSlotList)
	: owner(_Owner)
	, type(_Type)
	, isMultiSlot(_IsMultiSlot)
{
	ASSERT(owner != nullptr);
	node = NULL;
	name = _Name;
	if (AutoAddToSlotList) 
	{
		owner->slotList.push_back(this);
		owner->ReceiveMessage(this, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);
	}
}


Slot::~Slot()
{
	DisconnectAll(false);
}


bool Slot::Connect( Node* Nd )
{
	if (Nd && NodeType::ALLOW_ALL != type && Nd->GetType() != type)
	{
		/// TODO: use ASSERT only
		ERR("Slot and operator type mismatch");
		ASSERT(false);
		return false;
	}
	if (!isMultiSlot)
	{
		if (node != Nd)
		{
			if (node) node->DisconnectFromSlot(this);
			node = Nd;
			if (node) node->ConnectToSlot(this);

			owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
		}
	}
	else
	{
		ASSERT(Nd != nullptr);
		for (Node* node : multiNodes) {
			if (node == Nd) {
				ERR("Node already connected to slot.");
				return false;
			}
		}
		multiNodes.push_back(Nd);
		owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
	return true;
}


void Slot::Disconnect(Node* Nd)
{
	if (isMultiSlot) {
		for (auto it = multiNodes.begin(); it != multiNodes.end(); it++) {
			if (*it == Nd) {
				Nd->DisconnectFromSlot(this);
				multiNodes.erase(it);
				owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
				return;
			}
		}
		ERR("Node was not found.");
	}
	else 
	{
		ASSERT(Nd == node);
		ASSERT(Nd != nullptr);
		DisconnectAll(true);
	}
}


void Slot::DisconnectAll(bool NotifyOwner)
{
	if (isMultiSlot) {
		for (auto it = multiNodes.begin(); it != multiNodes.end(); it++) {
			(*it)->DisconnectFromSlot(this);
		}
		multiNodes.clear();
	}
	else
	{
		if (node) node->DisconnectFromSlot(this);
		node = nullptr;
	}

	if (NotifyOwner) {
		owner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
}


Node* Slot::GetNode() const 
{
	if (isMultiSlot) {
		ERR("Can't call GetNode() on multislot.");
		return nullptr;
	}
	return node;
}


const vector<Node*>& Slot::GetMultiNodes() const
{
	ASSERT(isMultiSlot);
	return multiNodes;
}


SharedString Slot::GetName()
{
	return name;
}


NodeType Slot::GetType() const
{
	return type;
}

void Slot::ChangeNodeIndex(Node* Nd, UINT TargetIndex)
{
	ASSERT(isMultiSlot);
	ASSERT(TargetIndex < multiNodes.size());

	auto it = std::find(multiNodes.begin(), multiNodes.end(), Nd);
	ASSERT(it != multiNodes.end());
	UINT index = it - multiNodes.begin();

	Node* node = multiNodes[index];
	if (index < TargetIndex) {
		for (UINT i = index; i < TargetIndex; i++) {
			multiNodes[i] = multiNodes[i + 1];
		}
		multiNodes[TargetIndex] = node;
	} else {
		for (UINT i = index; i > TargetIndex; i--) {
			multiNodes[i] = multiNodes[i - 1];
		}
		multiNodes[TargetIndex] = node;
	}
}

Node* Slot::operator[](UINT Index)
{
	ASSERT(isMultiSlot);
	ASSERT(Index < multiNodes.size());
	return multiNodes[Index];
}


Node::Node(NodeType _Type, const string& _Name)
	: name(_Name)
	, type(_Type)
{
	isDirty = true;
	isProperlyConnected = true;
}


Node::Node( const Node& Original )
	: name(Original.name)
	, type(Original.type)
{}


void Node::ConnectToSlot(Slot* S)
{
	/// No need for typecheck, the Slot already did that.
	dependants.push_back(S);
}


void Node::DisconnectFromSlot( Slot* S )
{
	dependants.erase(std::remove(dependants.begin(), dependants.end(), S), dependants.end());
}


const vector<Slot*>& Node::GetDependants() const
{
	return dependants;
}


NodeType Node::GetType() const
{
	return type;
}


void Node::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		CheckConnections();
		//SendMessage(NodeMessage::TRANSITIVE_CONNECTION_CHANGED);
		/// Fall through:
	case NodeMessage::VALUE_CHANGED:
		if (!isDirty)
		{
			isDirty = true;
			SendMessage(NodeMessage::VALUE_CHANGED);
		}
		break;
	default:
		break;
	}
}


void Node::SendMessage(NodeMessage Message, const void* Payload)
{
	for (Slot* slot : dependants) 
	{
		slot->owner->ReceiveMessage(slot, Message, Payload);
	}
}


void Node::ReceiveMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	HandleMessage(S, Message, Payload);
	onMessageReceived(S, Message, Payload);
}


void Node::CheckConnections()
{
	foreach (Slot* slot, slotList)
	{
		/// TODO: handle multislots
		if (!slot->isMultiSlot && slot->GetNode() == NULL)
		{
			isProperlyConnected = false;
			return;
		}
	}
	isProperlyConnected = true;
}


void Node::Evaluate()
{
	if (isDirty && isProperlyConnected)
	{
		foreach(Slot* slot, slotList) 
		{
			slot->GetNode()->Evaluate();
		}
		Operate();
		isDirty = false;
	}
}


Node::~Node()
{
	const vector<Slot*>& deps = GetDependants();
	while (deps.size())
	{
		deps.back()->Disconnect(this);
	}
}


Node* Node::Clone() const
{
	ASSERT(false);
	return NULL;
}


