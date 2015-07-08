#include "watcher.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"

Watcher::Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type)
    : Node(type)
    , mWatchedNode(NodeType::ALLOW_ALL, this, nullptr)
    , mWatcherWidget(watcherWidget) {
  if (watcherWidget) watcherWidget->mWatcher = this;
  mWatchedNode.Connect(node);
  MakeDisplayedName();
  node->onMessageReceived += Delegate(this, &Watcher::SniffMessage);
}

void Watcher::SniffMessage(Slot* slot, NodeMessage message, const void* payload) {
  if (message == NodeMessage::NODE_NAME_CHANGED) {
    MakeDisplayedName();
    if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
  }
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

void Watcher::ChangeNode(Node* node) {
  if (mWatchedNode.GetNode()) {
    mWatchedNode.GetNode()->onMessageReceived -= Delegate(this, &Watcher::SniffMessage);
    mWatchedNode.DisconnectAll(false);
  }
  if (node) {
    mWatchedNode.Connect(node);
    node->onMessageReceived += Delegate(this, &Watcher::SniffMessage);
  }
  MakeDisplayedName();
  HandleChangedNode(node);
}

void Watcher::HandleChangedNode(Node* node) {
  /// Overload this method if your watcher supports changing nodes.
  SHOULDNT_HAPPEN;
}

GLWidget* Watcher::GetGLWidget() {
  return mWatcherWidget->GetGLWidget();
}

void Watcher::MakeDisplayedName() {
  Node* node = GetNode();
  if (node == nullptr) {
    mDisplayedName = QString();
  } else if (!node->GetName().empty()) {
    /// Node has a name, use that.
    mDisplayedName = QString::fromStdString(node->GetName());
  } else {
    /// Just use the type as a name by default
    mDisplayedName = QString::fromStdString(
      NodeRegistry::GetInstance()->GetNodeClass(GetNode())->mClassName);
    if (node->GetType() == NodeType::SHADER_STUB) {
      ShaderStub* stub = static_cast<ShaderStub*>(node);
      ShaderStubMetadata* metaData = stub->GetStubMetadata();
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

void Watcher::SetName(const string& name) {
  SHOULDNT_HAPPEN;
}

const string& Watcher::GetName() const {
  SHOULDNT_HAPPEN;
  return Node::GetName();
}

void Watcher::SetPosition(const Vec2 position) {
  SHOULDNT_HAPPEN;
}

const Vec2 Watcher::GetPosition() const {
  SHOULDNT_HAPPEN;
  return Vec2(0, 0);
}

void Watcher::SetSize(const Vec2 size) {
  SHOULDNT_HAPPEN;
}

const Vec2 Watcher::GetSize() const {
  SHOULDNT_HAPPEN;
  return Vec2(0, 0);
}
