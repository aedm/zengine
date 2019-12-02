#include "watcherui.h"
#include "watcherwidget.h"
#include "../graph/prototypes.h"

WatcherUi::WatcherUi(const std::shared_ptr<Node>& node)
    : Watcher(node)
{
  mDisplayedName = CreateDisplayedName(GetDirectNode());
}


void WatcherUi::OnNameChange() {
  mDisplayedName = CreateDisplayedName(GetDirectNode());
  if (mWatcherWidget) mWatcherWidget->SetTabLabel(mDisplayedName);
}


WatcherUi::~WatcherUi() {
  ASSERT(mWatcherWidget == nullptr);
  ASSERT(GetDirectNode() == nullptr);
}


QGLWidget* WatcherUi::GetGlWidget() const
{
  return mWatcherWidget->GetGlWidget();
}


QString WatcherUi::CreateDisplayedName(const std::shared_ptr<Node>& node) {
  ASSERT(node != nullptr);

  if (!node->GetName().empty()) {
    /// Node has a name, use that.
    return QString::fromStdString(node->GetName());
  }

  const std::shared_ptr<Node> referencedNode = node->GetReferencedNode();
  if (referencedNode.use_count() == 0) return "Empty ghost";

  if (IsPointerOf<StubNode>(referencedNode)) {
    const std::shared_ptr<StubNode> stub = PointerCast<StubNode>(referencedNode);
    StubMetadata* metaData = stub->GetStubMetadata();
    if (metaData != nullptr && !metaData->mName.empty()) {
      /// For shader stubs, use the stub name by default
      return QString::fromStdString(metaData->mName);
    }
  }

  /// Just use the type as a name by default
  return QString::fromStdString(
    NodeRegistry::GetInstance()->GetNodeClass(referencedNode)->mClassName);
}


const QString& WatcherUi::GetDisplayedName() const
{
  return mDisplayedName;
}

void WatcherUi::SetWatcherWidget(WatcherWidget* watcherWidget) {
  /// This can only be set once
  ASSERT(!mWatcherWidget);
  mWatcherWidget = watcherWidget;
}

void WatcherUi::OnRemovedFromNode() {
  Watcher::OnRemovedFromNode();

  if (mWatcherWidget == nullptr) return;

  /// Call the Qt widget remover callback. A simple delete wouldn't do, because
  /// Qt handles tab widgets differently.
  WatcherWidget* tmp = mWatcherWidget;
  mWatcherWidget = nullptr;
  if (!mDeleteWatcherWidgetCallback.empty()) mDeleteWatcherWidgetCallback(tmp);

  /// point-of-no-return: the watcher widget destorys the last reference to this watcher
}
