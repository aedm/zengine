#include "graphCommands.h"
#include "../graph/nodewidget.h"

CreateNodeCommand::CreateNodeCommand( Node* _Nd, GraphEditor* _Panel )
	: Nd(_Nd)
	, Panel(_Panel)
{}

CreateNodeCommand::~CreateNodeCommand()
{
	if (!Active) {
		SafeDelete(Nd);
	}
}

bool CreateNodeCommand::Do()
{
	Panel->AddNode(Nd);
	return true;
}

bool CreateNodeCommand::Undo()
{
	NOT_IMPLEMENTED;
	return true;
}

MoveNodeCommand::MoveNodeCommand( NodeWidget* _NdWidget, const Vec2& _Position )
	: NdWidget(_NdWidget)
	, NewPosition(_Position)
{
	OldPosition = NdWidget->Position;
}

bool MoveNodeCommand::Do()
{
	NdWidget->SetPosition(NewPosition);
	return true;
}

bool MoveNodeCommand::Undo()
{
	NdWidget->SetPosition(OldPosition);
	return true;
}


ConnectNodeToSlotCommand::ConnectNodeToSlotCommand( Node* _FromNode, Slot* _ToSlot )
	: NewNode(_FromNode)
	, ToSlot(_ToSlot)
{
	OldNode = ToSlot->GetNode();
}

bool ConnectNodeToSlotCommand::Do()
{
	return ToSlot->Connect(NewNode);
}

bool ConnectNodeToSlotCommand::Undo()
{
	return ToSlot->Connect(OldNode);
}

DeleteNodeCommand::DeleteNodeCommand( const set<NodeWidget*>& _NodeWidgets )
	: NodeWidgets(_NodeWidgets)
{}

DeleteNodeCommand::~DeleteNodeCommand()
{
	if (Active) {
		foreach (NodeWidget* nw, NodeWidgets) {
			delete nw;
		}
	}
}

bool DeleteNodeCommand::Do()
{
	NOT_IMPLEMENTED
	foreach(NodeWidget* nw, NodeWidgets) {
		foreach (Slot* slot, nw->GetNode()->mSlots) {
			slot->Connect(NULL);
		}
	}
	return true;
}

bool DeleteNodeCommand::Undo()
{
	NOT_IMPLEMENTED
	return false;
}
