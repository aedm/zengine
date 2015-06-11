#include "watcher.h"
#include "watcherwidget.h"

Watcher::Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type)
    : Node(type, "")
    , mWatchedNode(NodeType::ALLOW_ALL, this, nullptr)
    , mWatcherWidget(watcherWidget) {
  if (watcherWidget) watcherWidget->mWatcher = this;
  mWatchedNode.Connect(node);
  node->onMessageReceived += Delegate(this, &Watcher::SniffMessage);
}

void Watcher::SniffMessage(Slot* slot, NodeMessage message, const void* payload) {
  HandleSniffedMessage(slot, message, payload);
}

Watcher::~Watcher() {
  if (mWatchedNode.GetNode()) {
    mWatchedNode.GetNode()->onMessageReceived -= Delegate(this, &Watcher::SniffMessage);
  }
}

void Watcher::HandleSniffedMessage(Slot*, NodeMessage, const void*) {}

Node* Watcher::GetNode() {
  return mWatchedNode.GetNode();
}


