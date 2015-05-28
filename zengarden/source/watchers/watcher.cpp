#include "watcher.h"
#include "watcherwidget.h"

Watcher::Watcher(Node* Nd, WatcherWidget* _Widget, NodeType Type)
	: Node(Type, "")
	, WatcherSlot(NodeType::ALLOW_ALL, this, nullptr)
	, Widget(_Widget)
{
	if (_Widget) _Widget->Watcher = this;
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


