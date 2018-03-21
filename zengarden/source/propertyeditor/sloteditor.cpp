#include "sloteditor.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QPushButton>

SlotEditor::SlotEditor(Node* node)
  : PropertyEditor(node) {}


SlotEditor::~SlotEditor() {
  for (auto it : mSlotWatchers) {
    if (it.second->GetNode()) it.second->GetNode()->RemoveWatcher(it.second.get());
    SafeDelete(it.second->mWatcherWidget);
  }
}


template <ValueType T>
void SlotEditor::AddSlot(Slot* slot) {
  if (!IsInsanceOf<ValueNode<T>*>(slot->GetReferencedNode())) return;

  auto valueSlot = SafeCast<ValueSlot<T>*>(slot);
  auto slotNode = valueSlot->GetReferencedNode();
  shared_ptr<SlotWatcher> watcher = slotNode->Watch<TypedSlotWatcher<T>>(valueSlot);

  WatcherWidget* widget =
    new WatcherWidget(mWatcherWidget, watcher, WatcherPosition::PROPERTY_PANEL);
  watcher->deleteWatcherWidgetCallback =
    Delegate(this, &SlotEditor::RemoveWatcherWidget);
  mLayout->addWidget(widget);
  watcher->SetWatcherWidget(widget);
  mSlotWatchers[slot] = watcher;
}


void SlotEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  /// Slots
  for (Slot* slot : mNode->GetPublicSlots()) {
    if (slot->mIsMultiSlot) continue;
    AddSlot<ValueType::FLOAT>(slot);
    AddSlot<ValueType::VEC2>(slot);
    AddSlot<ValueType::VEC3>(slot);
    AddSlot<ValueType::VEC4>(slot);
  }

  /// Source editor button
  if (IsExactType<StubNode>(mNode)) {
    StubNode* stub = static_cast<StubNode*>(mNode);
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
    Node* node = slot->GetReferencedNode();
    if (IsInsanceOf<StubNode*>(node)) {
      node->RemoveWatcher(watcher.get());
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


SlotWatcher::SlotWatcher(Node* node)
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

  auto valueNode = SafeCast<ValueNode<T>*>(mNode);
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
