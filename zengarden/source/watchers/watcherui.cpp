#include "watcherui.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"

WatcherUI::WatcherUI(Node* node, WatcherWidget* watcherWidget, NodeType type)
    : Watcher(node)
    , mWatcherWidget(watcherWidget) 
{
  if (watcherWidget) watcherWidget->mWatcher = this;
  MakeDisplayedName();
}


void WatcherUI::OnNameChange() {
  MakeDisplayedName();
  if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
}


WatcherUI::~WatcherUI() {
  if (mWatcherWidget) {
    /// Make sure someone will delete the WatcherWidget
    ASSERT(!mWatcherWidget->onWatcherDeath.empty());
    
    /// Don't let the WatcherWidget delete the Watcher.
    mWatcherWidget->mWatcher = nullptr;

    /// Let the UI delete the WatcherWidget
    mWatcherWidget->onWatcherDeath(mWatcherWidget);
  }
}


Node* WatcherUI::GetNode() {
  return mNode;
}


GLWidget* WatcherUI::GetGLWidget() {
  return mWatcherWidget->GetGLWidget();
}


void WatcherUI::MakeDisplayedName() {
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

const QString& WatcherUI::GetDisplayedName() {
  return mDisplayedName;
}
