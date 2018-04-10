#include "graphCommands.h"
#include "../graph/nodewidget.h"

CreateNodeCommand::CreateNodeCommand(Node* node, Graph* graph)
  : mNode(node)
  , mGraph(graph) {}

CreateNodeCommand::~CreateNodeCommand() {}

bool CreateNodeCommand::Do() {
  mGraph->mNodes.Connect(mNode);
  return true;
}

bool CreateNodeCommand::Undo() {
  NOT_IMPLEMENTED;
  return true;
}

MoveNodeCommand::MoveNodeCommand(Node* node, const Vec2& position)
  : mNode(node)
  , mNewPosition(position) {
  mOldPosition = mNode->GetPosition();
}

MoveNodeCommand::MoveNodeCommand(Node* node, const Vec2& position, 
                                 const Vec2& oldPosition) 
  : mNode(node)
  , mNewPosition(position)
  , mOldPosition(oldPosition)
{}

bool MoveNodeCommand::Do() {
  mNode->SetPosition(mNewPosition);
  return true;
}

bool MoveNodeCommand::Undo() {
  mNode->SetPosition(mOldPosition);
  return true;
}


ConnectNodeToSlotCommand::ConnectNodeToSlotCommand(Node* fromNode, Slot* slot)
  : mNewNode(fromNode)
  , mSlot(slot) {
  /// TODO: keep old, connected nodes (multinodes)
  //mOldNode = mSlot->GetAbstractNode();
}

bool ConnectNodeToSlotCommand::Do() {
  return mSlot->Connect(mNewNode);
}

bool ConnectNodeToSlotCommand::Undo() {
  return mSlot->Connect(mOldNode);
}

DeleteNodeCommand::DeleteNodeCommand(set<shared_ptr<Node>>& nodes)
  : mNodes(nodes) {}

DeleteNodeCommand::~DeleteNodeCommand() {}

bool DeleteNodeCommand::Do() {
  for (shared_ptr<Node> node : mNodes) {
    node->DisconnectAll();
  }
  mNodes.clear();
  return true;
}

bool DeleteNodeCommand::Undo() {
  NOT_IMPLEMENTED;
  return false;
}
