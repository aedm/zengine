#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeType _Type, Node* _Owner, SharedString _Name, bool _IsMultiSlot, 
	bool AutoAddToSlotList)
	: mOwner(_Owner)
	, mType(_Type)
	, mIsMultiSlot(_IsMultiSlot)
{
	ASSERT(mOwner != nullptr);
	mNode = NULL;
	mName = _Name;
	if (AutoAddToSlotList) 
	{
		mOwner->mSlots.push_back(this);
		mOwner->ReceiveMessage(this, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);
	}
}


Slot::~Slot()
{
	DisconnectAll(false);
}


bool Slot::Connect( Node* Nd )
{
	if (Nd && NodeType::ALLOW_ALL != mType && Nd->GetType() != mType)
	{
		/// TODO: use ASSERT only
		ERR("Slot and operator type mismatch");
		ASSERT(false);
		return false;
	}
	if (!mIsMultiSlot)
	{
		if (mNode != Nd)
		{
			if (mNode) mNode->DisconnectFromSlot(this);
			mNode = Nd;
			if (mNode) mNode->ConnectToSlot(this);

			mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
		}
	}
	else
	{
		ASSERT(Nd != nullptr);
		for (Node* node : mMultiNodes) {
			if (node == Nd) {
				ERR("Node already connected to slot.");
				return false;
			}
		}
		mMultiNodes.push_back(Nd);
		mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
	return true;
}


void Slot::Disconnect(Node* Nd)
{
	if (mIsMultiSlot) {
		for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); it++) {
			if (*it == Nd) {
				Nd->DisconnectFromSlot(this);
				mMultiNodes.erase(it);
				mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
				return;
			}
		}
		ERR("Node was not found.");
	}
	else 
	{
		ASSERT(Nd == mNode);
		ASSERT(Nd != nullptr);
		DisconnectAll(true);
	}
}


void Slot::DisconnectAll(bool NotifyOwner)
{
	if (mIsMultiSlot) {
		for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); it++) {
			(*it)->DisconnectFromSlot(this);
		}
		mMultiNodes.clear();
	}
	else
	{
		if (mNode) mNode->DisconnectFromSlot(this);
		mNode = nullptr;
	}

	if (NotifyOwner) {
		mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
	}
}


Node* Slot::GetNode() const 
{
	if (mIsMultiSlot) {
		ERR("Can't call GetNode() on multislot.");
		return nullptr;
	}
	return mNode;
}


const vector<Node*>& Slot::GetMultiNodes() const
{
	ASSERT(mIsMultiSlot);
	return mMultiNodes;
}


SharedString Slot::GetName()
{
	return mName;
}


NodeType Slot::GetType() const
{
	return mType;
}

void Slot::ChangeNodeIndex(Node* Nd, UINT TargetIndex)
{
	ASSERT(mIsMultiSlot);
	ASSERT(TargetIndex < mMultiNodes.size());

	auto it = std::find(mMultiNodes.begin(), mMultiNodes.end(), Nd);
	ASSERT(it != mMultiNodes.end());
	UINT index = it - mMultiNodes.begin();

	Node* node = mMultiNodes[index];
	if (index < TargetIndex) {
		for (UINT i = index; i < TargetIndex; i++) {
			mMultiNodes[i] = mMultiNodes[i + 1];
		}
		mMultiNodes[TargetIndex] = node;
	} else {
		for (UINT i = index; i > TargetIndex; i--) {
			mMultiNodes[i] = mMultiNodes[i - 1];
		}
		mMultiNodes[TargetIndex] = node;
	}
}

Node* Slot::operator[](UINT Index)
{
	ASSERT(mIsMultiSlot);
	ASSERT(Index < mMultiNodes.size());
	return mMultiNodes[Index];
}


Node::Node(NodeType _Type, const string& _Name)
	: mName(_Name)
	, mType(_Type)
{
	mIsDirty = true;
	mIsProperlyConnected = true;
}


Node::Node( const Node& Original )
	: mName(Original.mName)
	, mType(Original.mType)
{}


void Node::ConnectToSlot(Slot* S)
{
	/// No need for typecheck, the Slot already did that.
	mDependants.push_back(S);
}


void Node::DisconnectFromSlot( Slot* S )
{
	mDependants.erase(std::remove(mDependants.begin(), mDependants.end(), S), mDependants.end());
}


const vector<Slot*>& Node::GetDependants() const
{
	return mDependants;
}


NodeType Node::GetType() const
{
	return mType;
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
		if (!mIsDirty)
		{
			mIsDirty = true;
			SendMessage(NodeMessage::VALUE_CHANGED);
		}
		break;
	default:
		break;
	}
}


void Node::SendMessage(NodeMessage Message, const void* Payload)
{
	for (Slot* slot : mDependants) 
	{
		slot->mOwner->ReceiveMessage(slot, Message, Payload);
	}
}


void Node::ReceiveMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	HandleMessage(S, Message, Payload);
	onMessageReceived(S, Message, Payload);
}


void Node::CheckConnections()
{
	foreach (Slot* slot, mSlots)
	{
		/// TODO: handle multislots
		if (!slot->mIsMultiSlot && slot->GetNode() == NULL)
		{
			mIsProperlyConnected = false;
			return;
		}
	}
	mIsProperlyConnected = true;
}


void Node::Evaluate()
{
	if (mIsDirty && mIsProperlyConnected)
	{
		foreach(Slot* slot, mSlots) 
		{
			slot->GetNode()->Evaluate();
		}
		Operate();
		mIsDirty = false;
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


