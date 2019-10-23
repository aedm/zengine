#pragma once

#include "command.h"
#include "../graph/graphwatcher.h"

class CreateNodeCommand: public Command {
public:
  CreateNodeCommand(std::shared_ptr<Node> node, std::shared_ptr<Graph> graph);
  ~CreateNodeCommand();

  bool Do() override;
  bool Undo() override;

private:
  std::shared_ptr<Node> mNode;
  std::shared_ptr<Graph> mGraph;
};


class MoveNodeCommand: public Command {
public:
  MoveNodeCommand(std::shared_ptr<Node> node, const Vec2& position);
  MoveNodeCommand(std::shared_ptr<Node> node, const Vec2& position, 
    const Vec2& oldPosition);

  bool Do() override;
  bool Undo() override;

private:
  std::shared_ptr<Node> mNode;
  Vec2 mNewPosition;
  Vec2 mOldPosition;
};


class ConnectNodeToSlotCommand: public Command {
public:
  ConnectNodeToSlotCommand(std::shared_ptr<Node> fromNode, Slot* toSlot);

  bool Do() override;
  bool Undo() override;

private:
  std::shared_ptr<Node> mNewNode;
  std::shared_ptr<Node> mOldNode;
  Slot* mSlot;
};


class DisposeNodesCommand: public Command {
public:
  DisposeNodesCommand(std::vector<std::shared_ptr<Node>>& nodes);

  bool Do() override;
  bool Undo() override;

private:
  std::vector<std::shared_ptr<Node>> mNodes;
  std::shared_ptr<Graph> mGraph;
};