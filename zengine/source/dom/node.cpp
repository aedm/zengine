#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeType _Type, Node* _Owner, SharedString _Name )
	: Owner(_Owner)
	, Type(_Type)
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
	if (ConnectedNode != Nd)
	{
		if (Nd && NodeType::ALLOW_ALL != Type && Nd->GetType() != Type) 
		{
			ERR("Slot and operator type mismatch");
			ASSERT(false);
			return false;
		}

		if (ConnectedNode) ConnectedNode->DisconnectFromSlot(this);
		ConnectedNode = Nd;
		if (ConnectedNode) ConnectedNode->ConnectToSlot(this);
		
		if (Owner)
		{
			/// TODO: simplify this
			Owner->CheckConnections();
			Owner->OnSlotConnectionsChanged(this);
			Owner->OnSlotValueChanged(this);
		}
	}

	return true;
}


void Slot::DisconnectFromNode()
{
	ASSERT(ConnectedNode);
	ConnectedNode->DisconnectFromSlot(this);
	ConnectedNode = NULL;
}

Node* Slot::GetNode() const 
{
	return ConnectedNode;
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

void Node::OnSlotValueChanged(Slot* DirtySlot)
{
	if (!IsDirty)
	{
		IsDirty = true;
		SetDependantsDirty();
	}
}

void Node::SetDependantsDirty()
{
	foreach(Slot* dependant, Dependants)
	{
		dependant->GetNode()->OnSlotValueChanged(dependant);
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

