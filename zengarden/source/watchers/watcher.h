#pragma once

#include <zengine.h>

class Watcher : public Node
{
public:
	Watcher(Node* Nd, NodeType Type = NodeType::UI);
	virtual ~Watcher();

	Node*				GetNode();

protected:
	virtual void		HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload);
	Slot				WatcherSlot;

private:
	void				SniffMessage(Slot* S, NodeMessage Message, const void* Payload);

	virtual void		HandleMessage(Slot* S, NodeMessage Message, const void* Payload) override;
};