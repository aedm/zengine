#pragma once

#include "command.h"
#include "../graph/operatorPanel.h"

class CreateOperatorCommand: public Command
{
public:
	CreateOperatorCommand(Node* Nd, OperatorPanel* Panel);
	~CreateOperatorCommand();

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	Node*					Op;
	OperatorPanel*			Panel;
};


class MoveOperatorCommand: public Command
{
public:
	MoveOperatorCommand(OperatorWidget* OpWidget, const Vec2& Position);

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	OperatorWidget*			OpWidget;
	Vec2					NewPosition;
	Vec2					OldPosition;
};


class ConnectOperatorToSlotCommand: public Command
{
public:
	ConnectOperatorToSlotCommand(Node* FromOperator, Slot* ToSlot);

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	Node*					NewOperator;
	Node*					OldOperator;
	Slot*					ToSlot;
};


class DeleteOperatorCommand: public Command
{
public:
	DeleteOperatorCommand(const set<OperatorWidget*>& OpWidgets);
	virtual ~DeleteOperatorCommand();

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	set<OperatorWidget*>	OpWidgets;
};