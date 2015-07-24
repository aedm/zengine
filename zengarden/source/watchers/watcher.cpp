#include "watcher.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"

Watcher::Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type)
    : mNode(node)
    , mWatcherWidget(watcherWidget) {
  if (watcherWidget) watcherWidget->mWatcher = this;
  MakeDisplayedName();
  node->onMessageReceived += Delegate(this, &Watcher::SniffMessage);
}

void Watcher::SniffMessage(Slot* slot, NodeMessage message, void* payload) {
  if (message == NodeMessage::NODE_NAME_CHANGED) {
    MakeDisplayedName();
    if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
  }
  HandleSniffedMessage(slot, message, payload);
  if (message == NodeMessage::NODE_REMOVED) {
    ChangeNode(nullptr);
  }
}

Watcher::~Watcher() {
  if (mNode) {
    mNode->onMessageReceived -= Delegate(this, &Watcher::SniffMessage);
    mNode = nullptr;
  }
}

void Watcher::HandleSniffedMessage(Slot*, NodeMessage, void*) {}

Node* Watcher::GetNode() {
  return mNode;
}

void Watcher::ChangeNode(Node* node) {
  if (mNode) {
    mNode->onMessageReceived -= Delegate(this, &Watcher::SniffMessage);
  }
  mNode = node;
  if (node) {
    node->onMessageReceived += Delegate(this, &Watcher::SniffMessage);
  }
  MakeDisplayedName();
  HandleChangedNode(node);
}

void Watcher::HandleChangedNode(Node* node) {
}

GLWidget* Watcher::GetGLWidget() {
  return mWatcherWidget->GetGLWidget();
}

void Watcher::MakeDisplayedName() {
  if (mNode == nullptr) {
    mDisplayedName = QString();
  } else if (!mNode->GetName().empty()) {
    /// Node has a name, use that.
    mDisplayedName = QString::fromStdString(mNode->GetName());
  } else {
    /// Just use the type as a name by default
    mDisplayedName = QString::fromStdString(
      NodeRegistry::GetInstance()->GetNodeClass(mNode)->mClassName);
    if (mNode->GetType() == NodeType::SHADER_STUB) {
      StubNode* stub = static_cast<StubNode*>(mNode);
      StubMetadata* metaData = stub->GetStubMetadata();
      if (metaData != nullptr && !metaData->name.empty()) {
        /// For shader stubs, use the stub name by default
        mDisplayedName = QString::fromStdString(metaData->name);
      }
    }
  }
}

const QString& Watcher::GetDisplayedName() {
  return mDisplayedName;
}
