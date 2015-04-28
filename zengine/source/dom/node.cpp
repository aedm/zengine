#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeType _Type, Node* _Owner, SharedString _Name, bool _IsMultiSlot)
	: Owner(_Owner)
	, Type(_Type)
	, IsMultiSlot(_IsMultiSlot)
{
	ConnectedNode = NULL;
	Name = _Name;
}


Slot::~Slot()
{
	Connect(NULL);
}


bool Slot::Connect( Node* Nd )
{
	if (Nd && NodeType::ALLOW_ALL != Type && Nd->GetType() != Type)
	{
		ERR("Slot and operator type mismatch");
		ASSERT(false);
		return false;
	}
	if (!IsMultiSlot)
	{
		if (ConnectedNode != Nd)
		{
			if (ConnectedNode) ConnectedNode->DisconnectFromSlot(this);
			ConnectedNode = Nd;
			if (ConnectedNode) ConnectedNode->ConnectToSlot(this);

			Owner->HandleMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
		}
	}
	else
	{
		ASSERT(Nd != nullptr);
		for (Node* node : MultiNodes) {
			if (node == Nd) {
				ERR("Node already connected to slot.");
				return false;
			}
		}
		MultiNodes.push_back(Nd);
		Owner->HandleMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
	return true;
}


void Slot::Disconnect(Node* Nd)
{
	if (IsMultiSlot) {
		for (auto it = MultiNodes.begin(); it != MultiNodes.end(); it++) {
			if (*it == Nd) {
				Nd->DisconnectFromSlot(this);
				MultiNodes.erase(it);
				Owner->HandleMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
				return;
			}
		}
		ERR("Node was not found.");
	}
	else 
	{
		ASSERT(Nd == ConnectedNode);
		ASSERT(Nd != nullptr);
		DisconnectAll();
	}
}


void Slot::DisconnectAll()
{
	if (IsMultiSlot) {
		for (auto it = MultiNodes.begin(); it != MultiNodes.end(); it++) {
			(*it)->DisconnectFromSlot(this);
		}
		MultiNodes.clear();
		Owner->HandleMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
	else
	{
		if (ConnectedNode) ConnectedNode->DisconnectFromSlot(this);
		ConnectedNode = nullptr;
		Owner->HandleMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
}


Node* Slot::GetNode() const 
{
	if (IsMultiSlot) {
		ERR("Can't call GetNode() on multislot.");
		return nullptr;
	}
	return ConnectedNode;
}


const vector<Node*>* Slot::GetMultiNodes() const
{
	if (!IsMultiSlot) {
		ERR("Can't call GetMultiNodes() on non-multislot.");
		return nullptr;
	}
	return &MultiNodes;
}


SharedString Slot::GetName()
{
	return Name;
}


NodeType Slot::GetType() const
{
	return Type;
}


Node::Node(NodeType _Type, const string& _Name)
	: Name(_Name)
	, Type(_Type)
{
	IsDirty = true;
	IsProperlyConnected = true;
}


Node::Node( const Node& Original )
	: Name(Original.Name)
	, Type(Original.Type)
{}


void Node::ConnectToSlot(Slot* S)
{
	/// No need for typecheck, the Slot already did that.
	Dependants.push_back(S);
}


void Node::DisconnectFromSlot( Slot* S )
{
	Dependants.erase(std::remove(Dependants.begin(), Dependants.end(), S), Dependants.end());
}


const vector<Slot*>& Node::GetDependants() const
{
	return Dependants;
}


NodeType Node::GetType() const
{
	return Type;
}


void Node::HandleMessage(Slot* S, NodeMessage Message)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		CheckConnections();
		/// Fall through:
	case NodeMessage::VALUE_CHANGED:
		if (!IsDirty)
		{
			IsDirty = true;
			SendMessage(NodeMessage::VALUE_CHANGED);
		}
		break;
	case NodeMessage::NEEDS_REDRAW:
		/// Only watchers need to handle this.
		break;
	default:
		WARN("Unhandled message: ", (UINT)Message);
		break;
	}
}


void Node::SendMessage(NodeMessage Message)
{
	for (Slot* slot : Dependants) 
	{
		slot->GetNode()->HandleMessage(slot, Message);
	}
}


void Node::CheckConnections()
{
	foreach (Slot* slot, Slots)
	{
		if (slot->GetNode() == NULL)
		{
			IsProperlyConnected = false;
			return;
		}
	}
	IsProperlyConnected = true;
}


void Node::Evaluate()
{
	if (IsDirty && IsProperlyConnected)
	{
		foreach(Slot* slot, Slots) 
		{
			slot->GetNode()->Evaluate();
		}
		Operate();
		IsDirty = false;
	}
}


Node::~Node()
{
	const vector<Slot*>& deps = GetDependants();
	while (deps.size())
	{
		deps.back()->Connect(NULL);
	}
}


Node* Node::Clone() const
{
	ASSERT(false);
	return NULL;
}


