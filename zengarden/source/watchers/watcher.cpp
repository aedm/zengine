#include "watcher.h"

Watcher::Watcher(Node* Nd, NodeType Type)
	: Node(Type, "")
	, WatcherSlot(NodeType::ALLOW_ALL, this, nullptr)
{
	WatcherSlot.Connect(Nd);
	Nd->OnMessageReceived += Delegate(this, &Watcher::SniffMessage);
}

void Watcher::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{}

void Watcher::SniffMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	HandleSniffedMessage(S, Message, Payload);
}

Watcher::~Watcher()
{
	if (WatcherSlot.GetNode()) {
		WatcherSlot.GetNode()->OnMessageReceived -= Delegate(this, &Watcher::SniffMessage);
	}
}

void Watcher::HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload)
{}

Node* Watcher::GetNode()
{
	return WatcherSlot.GetNode();
}


