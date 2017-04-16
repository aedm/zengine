#include "watcherui.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"
#include "../zengarden.h"

WatcherUI::WatcherUI(Node* node)
    : Watcher(node)
{
  MakeDisplayedName();
}


void WatcherUI::OnNameChange() {
  MakeDisplayedName();
  if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
}


WatcherUI::~WatcherUI() {
  ASSERT(mWatcherWidget == nullptr);
  ASSERT(mNode == nullptr);
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

void WatcherUI::SetWatcherWidget(WatcherWidget* watcherWidget) {
  /// This can only be set once
  ASSERT(!mWatcherWidget);
  mWatcherWidget = watcherWidget;
}

void WatcherUI::Unwatch() {
  Watcher::Unwatch();
  if (!mWatcherWidget) return;

  // Point of no return -- "this" pointer might be invalid after calling onUnwatch
  onUnwatch(mWatcherWidget);
}
