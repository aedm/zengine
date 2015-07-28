#include "watcher.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"

Watcher::Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type)
    : mNode(node)
    , mWatcherWidget(watcherWidget) {
  if (watcherWidget) watcherWidget->mWatcher = this;
  MakeDisplayedName();
  node->onSniffMessage += Delegate(this, &Watcher::SniffMessage);
}

void Watcher::SniffMessage(NodeMessage message, Slot* slot, void* payload) {
  if (message == NodeMessage::NODE_NAME_CHANGED) {
    MakeDisplayedName();
    if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
  }
  HandleSniffedMessage(message, slot, payload);
  if (message == NodeMessage::NODE_REMOVED) {
    ChangeNode(nullptr);
    if (mWatcherWidget) {
      ASSERT(!mWatcherWidget->onWatcherDeath.empty());
      mWatcherWidget->onWatcherDeath(mWatcherWidget);
      /// After this point, the "this" pointer is invalid.
    }
  }
}

Watcher::~Watcher() {
  if (mNode) {
    mNode->onSniffMessage -= Delegate(this, &Watcher::SniffMessage);
    mNode = nullptr;
  }
}

void Watcher::HandleSniffedMessage(NodeMessage, Slot*, void*) {}

Node* Watcher::GetNode() {
  return mNode;
}

void Watcher::ChangeNode(Node* node) {
  if (mNode) {
    mNode->onSniffMessage -= Delegate(this, &Watcher::SniffMessage);
  }
  mNode = node;
  if (node) {
    node->onSniffMessage += Delegate(this, &Watcher::SniffMessage);
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
