#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeTypeEnum _Type, Node* _Owner, SharedString _Name )
	: Owner(_Owner)
	, Type(_Type)
{
	AttachedOperator = NULL;
	Name = _Name;
}

Slot::~Slot()
{
	Connect(NULL);
}

bool Slot::Connect( Node* Op )
{
	if (AttachedOperator != Op)
	{
		if (Op && Op->GetType() != GetType()) 
		{
			ERR("Slot and operator type mismatch");
			ASSERT(false);
			return false;
		}

		if (AttachedOperator) AttachedOperator->DisconnectFromSlot(this);
		AttachedOperator = Op;
		if (AttachedOperator) AttachedOperator->ConnectToSlot(this);
		
		FinalizeAttach();
	}

	return true;
}


void Slot::DisconnectFromOperator()
{
	ASSERT(AttachedOperator);
	AttachedOperator->DisconnectFromSlot(this);
	AttachedOperator = NULL;
}


void Slot::FinalizeAttach()
{
	if (Owner)
	{
		Owner->CheckConnections();
		Owner->OnSlotConnectionsChanged(this);
		Owner->OnSlotValueChanged(this);
	}
}


Node* Slot::GetAttachedOperator() const 
{
	return AttachedOperator;
}

SharedString Slot::GetName()
{
	return Name;
}

NodeTypeEnum Slot::GetType() const
{
	return Type;
}

Node::Node(const string& _Name)
	: Name(_Name)
	, Type(OP_UNDEFINED)
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

NodeTypeEnum Node::GetType() const
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
		dependant->GetAttachedOperator()->OnSlotValueChanged(dependant);
	}
}

void Node::CheckConnections()
{
	foreach (Slot* slot, Slots)
	{
		if (slot->GetAttachedOperator() == NULL)
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
			slot->GetAttachedOperator()->Evaluate();
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

bool Node::CanSetValueDirectly()
{
	return false;
}

Node* Node::Clone() const
{
	ASSERT(false);
	return NULL;
}

