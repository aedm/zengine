#pragma once

#include "command.h"
#include "../graph/graphwatcher.h"

class CreateNodeCommand: public Command {
public:
  CreateNodeCommand(Node* node, GraphWatcher* graphWatcher);
  ~CreateNodeCommand();

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  Node* mNode;
  GraphWatcher* mGraphWatcher;
};


class MoveNodeCommand: public Command {
public:
  MoveNodeCommand(Node* node, const Vec2& position);
  MoveNodeCommand(Node* node, const Vec2& position, const Vec2& oldPosition);

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  Node* mNode;
  Vec2 mNewPosition;
  Vec2 mOldPosition;
};


class ConnectNodeToSlotCommand: public Command {
public:
  ConnectNodeToSlotCommand(Node* fromNode, Slot* toSlot);

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  Node* mNewNode;
  Node* mOldNode;
  Slot* mSlot;
};


class DeleteNodeCommand: public Command {
public:
  DeleteNodeCommand(const set<NodeWidget*>& nodeWidgets);
  virtual ~DeleteNodeCommand();

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  set<NodeWidget*> mNodeWidgets;
};