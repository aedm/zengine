#pragma once

#include "command.h"
#include "../graph/graphwatcher.h"

class CreateNodeCommand: public Command {
public:
  CreateNodeCommand(shared_ptr<Node> node, shared_ptr<Graph> graph);
  ~CreateNodeCommand();

  bool Do() override;
  bool Undo() override;

private:
  shared_ptr<Node> mNode;
  shared_ptr<Graph> mGraph;
};


class MoveNodeCommand: public Command {
public:
  MoveNodeCommand(shared_ptr<Node> node, const Vec2& position);
  MoveNodeCommand(shared_ptr<Node> node, const Vec2& position, 
    const Vec2& oldPosition);

  bool Do() override;
  bool Undo() override;

private:
  shared_ptr<Node> mNode;
  Vec2 mNewPosition;
  Vec2 mOldPosition;
};


class ConnectNodeToSlotCommand: public Command {
public:
  ConnectNodeToSlotCommand(shared_ptr<Node> fromNode, Slot* toSlot);

  bool Do() override;
  bool Undo() override;

private:
  shared_ptr<Node> mNewNode;
  shared_ptr<Node> mOldNode;
  Slot* mSlot;
};


class DisposeNodesCommand: public Command {
public:
  DisposeNodesCommand(vector<shared_ptr<Node>>& nodes);

  bool Do() override;
  bool Undo() override;

private:
  vector<shared_ptr<Node>> mNodes;
  shared_ptr<Graph> mGraph;
};