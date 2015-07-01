#pragma once

#include "command.h"
#include "../graph/graphwatcher.h"

class CreateNodeCommand: public Command
{
public:
	CreateNodeCommand(Node* Nd, GraphWatcher* Panel);
	~CreateNodeCommand();

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	Node*					Nd;
	GraphWatcher*			Panel;
};


class MoveNodeCommand: public Command
{
public:
	MoveNodeCommand(NodeWidget* OpWidget, const Vec2& Position);

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	NodeWidget*				NdWidget;
	Vec2					NewPosition;
	Vec2					OldPosition;
};


class ConnectNodeToSlotCommand: public Command
{
public:
	ConnectNodeToSlotCommand(Node* FromNode, Slot* ToSlot);

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	Node*					NewNode;
	Node*					OldNode;
	Slot*					ToSlot;
};


class DeleteNodeCommand: public Command
{
public:
	DeleteNodeCommand(const set<NodeWidget*>& NodeWidgets);
	virtual ~DeleteNodeCommand();

	virtual bool			Do() override;
	virtual bool			Undo() override;

private:
	set<NodeWidget*>		NodeWidgets;
};