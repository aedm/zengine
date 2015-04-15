#include "graphCommands.h"
#include "../graph/operatorWidget.h"

CreateOperatorCommand::CreateOperatorCommand( Node* _Nd, OperatorPanel* _Panel )
	: Op(_Nd)
	, Panel(_Panel)
{}

CreateOperatorCommand::~CreateOperatorCommand()
{
	if (!Active) {
		SafeDelete(Op);
	}
}

bool CreateOperatorCommand::Do()
{
	Panel->AddOperator(Op);
	return true;
}

bool CreateOperatorCommand::Undo()
{
	NOT_IMPLEMENTED;
	return true;
}

MoveOperatorCommand::MoveOperatorCommand( OperatorWidget* _OpWidget, const Vec2& _Position )
	: OpWidget(_OpWidget)
	, NewPosition(_Position)
{
	OldPosition = OpWidget->Position;
}

bool MoveOperatorCommand::Do()
{
	OpWidget->SetPosition(NewPosition);
	return true;
}

bool MoveOperatorCommand::Undo()
{
	OpWidget->SetPosition(OldPosition);
	return true;
}


ConnectOperatorToSlotCommand::ConnectOperatorToSlotCommand( Node* _FromOperator, Slot* _ToSlot )
	: NewOperator(_FromOperator)
	, ToSlot(_ToSlot)
{
	OldOperator = ToSlot->GetAttachedOperator();
}

bool ConnectOperatorToSlotCommand::Do()
{
	return ToSlot->Connect(NewOperator);
}

bool ConnectOperatorToSlotCommand::Undo()
{
	return ToSlot->Connect(OldOperator);
}

DeleteOperatorCommand::DeleteOperatorCommand( const set<OperatorWidget*>& _OpWidgets )
	: OpWidgets(_OpWidgets)
{}

DeleteOperatorCommand::~DeleteOperatorCommand()
{
	if (Active) {
		foreach (OperatorWidget* ow, OpWidgets) {
			delete ow;
		}
	}
}

bool DeleteOperatorCommand::Do()
{
	NOT_IMPLEMENTED
	foreach(OperatorWidget* ow, OpWidgets) {
		foreach (Slot* slot, ow->GetOperator()->Slots) {
			slot->Connect(NULL);
		}
	}
	return true;
}

bool DeleteOperatorCommand::Undo()
{
	NOT_IMPLEMENTED
	return false;
}
