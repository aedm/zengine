#include "graphCommands.h"
#include "../graph/nodewidget.h"

CreateNodeCommand::CreateNodeCommand(Node* node, GraphWatcher* graphWatcher)
  : mNode(node)
  , mGraphWatcher(graphWatcher) {}

CreateNodeCommand::~CreateNodeCommand() {
  if (!mIsActive) {
    SafeDelete(mNode);
  }
}

bool CreateNodeCommand::Do() {
  mGraphWatcher->AddNode(mNode);
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
  mOldNode = mSlot->GetNode();
}

bool ConnectNodeToSlotCommand::Do() {
  return mSlot->Connect(mNewNode);
}

bool ConnectNodeToSlotCommand::Undo() {
  return mSlot->Connect(mOldNode);
}

DeleteNodeCommand::DeleteNodeCommand(const set<NodeWidget*>& nodeWidgets)
  : mNodeWidgets(nodeWidgets) {}

DeleteNodeCommand::~DeleteNodeCommand() {
  if (mIsActive) {
    for(NodeWidget* widget : mNodeWidgets) {
      delete widget;
    }
  }
}

bool DeleteNodeCommand::Do() {
  NOT_IMPLEMENTED;
  for (NodeWidget* widget : mNodeWidgets) {
    for (Slot* slot : widget->GetNode()->mSlots) {
      slot->Connect(NULL);
    }
  }
  return true;
}

bool DeleteNodeCommand::Undo() {
  NOT_IMPLEMENTED;
  return false;
}
