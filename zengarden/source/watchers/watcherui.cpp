#include "watcherui.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"
#include "../zengarden.h"

WatcherUI::WatcherUI(Node* node)
    : Watcher(node)
{
  mDisplayedName = CreateDisplayedName(mNode);
}


void WatcherUI::OnNameChange() {
  mDisplayedName = CreateDisplayedName(mNode);
  if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
}


WatcherUI::~WatcherUI() {
  ASSERT(mWatcherWidget == nullptr);
  ASSERT(mNode == nullptr);
}


EventForwarderGLWidget* WatcherUI::GetGLWidget() {
  return mWatcherWidget->GetGLWidget();
}


QString WatcherUI::CreateDisplayedName(Node* node) {
  if (node == nullptr) {
    return QString();
  } 
  
  if (!node->GetName().empty()) {
    /// Node has a name, use that.
    return QString::fromStdString(node->GetName());
  } 
  
  if (IsInsanceOf<StubNode*>(node)) {
    StubNode* stub = static_cast<StubNode*>(node);
    StubMetadata* metaData = stub->GetStubMetadata();
    if (metaData != nullptr && !metaData->name.empty()) {
      /// For shader stubs, use the stub name by default
      return QString::fromStdString(metaData->name);
    }
  }

  /// Just use the type as a name by default
  return QString::fromStdString(
    NodeRegistry::GetInstance()->GetNodeClass(node)->mClassName);
}


const QString& WatcherUI::GetDisplayedName() {
  return mDisplayedName;
}

void WatcherUI::SetWatcherWidget(WatcherWidget* watcherWidget) {
  /// This can only be set once
  ASSERT(!mWatcherWidget);
  mWatcherWidget = watcherWidget;
}

void WatcherUI::OnDeleteNode() {
  if (mWatcherWidget) {
    WatcherWidget* widget = mWatcherWidget;
    mWatcherWidget = nullptr;
    widget->mWatcher = nullptr;

    /// Call the function that deletes the watcher widget
    if (deleteWatcherWidgetCallback) deleteWatcherWidgetCallback(widget);
  }
}
