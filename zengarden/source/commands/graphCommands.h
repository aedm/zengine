#pragma once

#include "command.h"
#include "../graph/graphwatcher.h"

class CreateNodeCommand: public Command {
public:
  CreateNodeCommand(Node* node, Graph* graph);
  ~CreateNodeCommand();

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  shared_ptr<Node> mNode;
  shared_ptr<Graph> mGraph;
};


class MoveNodeCommand: public Command {
public:
  MoveNodeCommand(Node* node, const Vec2& position);
  MoveNodeCommand(Node* node, const Vec2& position, const Vec2& oldPosition);

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  shared_ptr<Node> mNode;
  Vec2 mNewPosition;
  Vec2 mOldPosition;
};


class ConnectNodeToSlotCommand: public Command {
public:
  ConnectNodeToSlotCommand(Node* fromNode, Slot* toSlot);

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  shared_ptr<Node> mNewNode;
  shared_ptr<Node> mOldNode;
  Slot* mSlot;
};


class DeleteNodeCommand: public Command {
public:
  DeleteNodeCommand(set<shared_ptr<Node>>& nodes);
  virtual ~DeleteNodeCommand();

  virtual bool Do() override;
  virtual bool Undo() override;

private:
  set<shared_ptr<Node>> mNodes;
};