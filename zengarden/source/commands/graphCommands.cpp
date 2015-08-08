#include "graphCommands.h"
#include "../graph/nodewidget.h"

CreateNodeCommand::CreateNodeCommand(Node* node, Graph* graph)
  : mNode(node)
  , mGraph(graph) {}

CreateNodeCommand::~CreateNodeCommand() {
  if (!mIsActive) {
    SafeDelete(mNode);
  }
}

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

DeleteNodeCommand::DeleteNodeCommand(OWNERSHIP set<Node*>* nodes)
  : mNodes(nodes) {}

DeleteNodeCommand::~DeleteNodeCommand() {
  //if (mIsActive) {
  //  for (Node* node : *mNodes) {
  //    delete node;
  //  }
  //}
}

bool DeleteNodeCommand::Do() {
  for (Node* node : *mNodes) {
    /// TODO: don't delete them, just remove them from the graph.
    delete node;
  }
  SafeDelete(mNodes);
  return true;
}

bool DeleteNodeCommand::Undo() {
  NOT_IMPLEMENTED;
  return false;
}
