#include "sloteditor.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

SlotEditor::SlotEditor(const shared_ptr<Node>& node)
  : PropertyEditor(node) 
{
  mGhostIcon.addFile(QStringLiteral(":/zengarden/icons/angle-right.svg"), 
    QSize(), QIcon::Normal, QIcon::On);
}


SlotEditor::~SlotEditor() {
  for (auto it : mSlotWatchers) {
    if (it.second->GetDirectNode()) {
      it.second->GetDirectNode()->RemoveWatcher(it.second);
    }
    SafeDelete(it.second->mWatcherWidget);
  }
}


template <ValueType T>
bool SlotEditor::AddSlot(Slot* slot, QWidget* parent, QLayout* layout) {
  if (!IsPointerOf<ValueNode<T>>(slot->GetReferencedNode())) return false;

  auto valueSlot = SafeCast<ValueSlot<T>*>(slot);
  auto slotNode = valueSlot->GetReferencedNode();
  shared_ptr<SlotWatcher> watcher = slotNode->Watch<TypedSlotWatcher<T>>(valueSlot);

  WatcherWidget* widget =
    new WatcherWidget(parent, watcher, WatcherPosition::PROPERTY_PANEL);
  watcher->deleteWatcherWidgetCallback =
    Delegate(this, &SlotEditor::RemoveWatcherWidget);
  layout->addWidget(widget);
  watcher->SetWatcherWidget(widget);
  mSlotWatchers[slot] = watcher;
  return true;
}


void SlotEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);
  shared_ptr<Node> directNode = GetDirectNode();

  /// Slots
  for (Slot* slot : directNode->GetPublicSlots()) {
    if (slot->mIsMultiSlot) continue;

    /// Create horizontal widget to add editor and ghost button
    QWidget* widget = new QWidget(watcherWidget);
    mLayout->addWidget(widget);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QHBoxLayout* hLayout = new QHBoxLayout(widget);
    hLayout->setSpacing(4);
    hLayout->setContentsMargins(0, 0, 0, 0);

    if (!AddSlot<ValueType::FLOAT>(slot, watcherWidget, hLayout) &&
      !AddSlot<ValueType::VEC2>(slot, watcherWidget, hLayout) &&
      !AddSlot<ValueType::VEC3>(slot, watcherWidget, hLayout) &&
      !AddSlot<ValueType::VEC4>(slot, watcherWidget, hLayout)) {
      QLabel* label = new QLabel(QString::fromStdString(*slot->GetName().get()), widget);
      hLayout->addWidget(label);
    }

    QPushButton* ghostButton = new QPushButton("", widget);
    ghostButton->setCheckable(true);
    ghostButton->setChecked(slot->IsGhost());
    ghostButton->setFlat(true);
    ghostButton->setMaximumWidth(20);
    ghostButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    ghostButton->setIcon(mGhostIcon);
    hLayout->addWidget(ghostButton);
    
    ghostButton->connect(ghostButton, &QPushButton::toggled, [=]() {
      slot->SetGhost(!slot->IsGhost());
    });
  }

  /// Source editor button
  if (IsExactType<StubNode>(directNode)) {
    auto stub = PointerCast<StubNode>(directNode);
    QPushButton* sourceButton = new QPushButton("Edit source", watcherWidget);
    watcherWidget->connect(sourceButton, &QPushButton::pressed, [=]() {
      ZenGarden::GetInstance()->Watch(
        stub->mSource.GetReferencedNode(), WatcherPosition::RIGHT_TAB);
    });
    mLayout->addWidget(sourceButton);
  }
}

void SlotEditor::OnSlotConnectionChanged(Slot* slot) {
  auto it = mSlotWatchers.find(slot);
  if (it != mSlotWatchers.end()) {
    shared_ptr<SlotWatcher> watcher = it->second;
    shared_ptr<Node> node = slot->GetReferencedNode();
    if (IsPointerOf<StubNode>(node)) {
      node->RemoveWatcher(watcher);
    }
    else {
      slot->GetReferencedNode()->AssignWatcher(watcher);
      SlotWatcher* watcherPtr = watcher.get();
      watcherPtr->UpdateReadOnly();
    }
  }
}


void SlotEditor::RemoveWatcherWidget(WatcherWidget* watcherWidget) {
  delete watcherWidget;
}


SlotWatcher::SlotWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node) {}


template <ValueType T>
TypedSlotWatcher<T>::TypedSlotWatcher(ValueSlot<T>* slot)
  : SlotWatcher(slot->GetReferencedNode())
  , mSlot(slot) 
{}


template <ValueType T>
void TypedSlotWatcher<T>::SetWatcherWidget(WatcherWidget* watcherWidget) {
  SlotWatcher::SetWatcherWidget(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(watcherWidget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  shared_ptr<ValueNode<T>> valueNode = PointerCast<ValueNode<T>>(GetNode());
  auto value = valueNode->Get();

  mEditor = new ValueEditor<T>(watcherWidget, 
    QString::fromStdString(*mSlot->GetName().get()), value);
  mEditor->onValueChange += Delegate(this, &TypedSlotWatcher<T>::HandleValueChange);

  layout->addWidget(mEditor);

  Vec2 range = mSlot->GetRange();
  mEditor->SetRange(range.x, range.y);
  UpdateReadOnly();
}


template <ValueType T>
void TypedSlotWatcher<T>::UpdateReadOnly() {
  mEditor->SetReadOnly(!mSlot->IsDefaulted());
}


template <ValueType T>
void TypedSlotWatcher<T>::HandleValueChange(QWidget* widget, const Type& value) {
  mSlot->SetDefaultValue(value);
}


template class TypedSlotWatcher<ValueType::FLOAT>;
template class TypedSlotWatcher<ValueType::VEC2>;
template class TypedSlotWatcher<ValueType::VEC3>;
template class TypedSlotWatcher<ValueType::VEC4>;
