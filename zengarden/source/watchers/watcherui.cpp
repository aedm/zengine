#include "watcherui.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"
#include "../zengarden.h"

WatcherUI::WatcherUI(const shared_ptr<Node>& node)
    : Watcher(node)
{
  mDisplayedName = CreateDisplayedName(GetDirectNode());
}


void WatcherUI::OnNameChange() {
  mDisplayedName = CreateDisplayedName(GetDirectNode());
  if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
}


WatcherUI::~WatcherUI() {
  ASSERT(mWatcherWidget == nullptr);
  ASSERT(GetDirectNode() == nullptr);
}


EventForwarderGLWidget* WatcherUI::GetGLWidget() const
{
  return mWatcherWidget->GetGLWidget();
}


QString WatcherUI::CreateDisplayedName(const shared_ptr<Node>& node) {
  ASSERT(node != nullptr);

  if (!node->GetName().empty()) {
    /// Node has a name, use that.
    return QString::fromStdString(node->GetName());
  }

  const shared_ptr<Node> referencedNode = node->GetReferencedNode();
  if (referencedNode.use_count() == 0) return "Empty ghost";

  if (IsPointerOf<StubNode>(referencedNode)) {
    const shared_ptr<StubNode> stub = PointerCast<StubNode>(referencedNode);
    StubMetadata* metaData = stub->GetStubMetadata();
    if (metaData != nullptr && !metaData->name.empty()) {
      /// For shader stubs, use the stub name by default
      return QString::fromStdString(metaData->name);
    }
  }

  /// Just use the type as a name by default
  return QString::fromStdString(
    NodeRegistry::GetInstance()->GetNodeClass(referencedNode)->mClassName);
}


const QString& WatcherUI::GetDisplayedName() const
{
  return mDisplayedName;
}

void WatcherUI::SetWatcherWidget(WatcherWidget* watcherWidget) {
  /// This can only be set once
  ASSERT(!mWatcherWidget);
  mWatcherWidget = watcherWidget;
}

void WatcherUI::OnRemovedFromNode() {
  Watcher::OnRemovedFromNode();

  if (mWatcherWidget == nullptr) return;

  /// Call the Qt widget remover callback. A simple delete wouldn't do, because
  /// Qt handles tab widgets differently.
  WatcherWidget* tmp = mWatcherWidget;
  mWatcherWidget = nullptr;
  if (!deleteWatcherWidgetCallback.empty()) deleteWatcherWidgetCallback(tmp);

  /// point-of-no-return: the watcher widget destorys the last reference to this watcher
}
