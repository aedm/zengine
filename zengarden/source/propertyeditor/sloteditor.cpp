#include "sloteditor.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QClipboard>

SlotEditor::SlotEditor(const shared_ptr<Node>& node)
  : PropertyEditor(node) 
{
  mGhostIcon.addFile(QStringLiteral(":/zengarden/icons/angle-right.svg"), 
    QSize(), QIcon::Normal, QIcon::On);
}


SlotEditor::~SlotEditor() {
  RemoveAllSlots();
}


template <ValueType T>
bool SlotEditor::AddSlot(Slot* slot, QWidget* parent, QLayout* layout) {
  if (!IsPointerOf<ValueNode<ValueTypes<T>::Type>>(slot->GetReferencedNode())) {
    return false;
  }

  auto valueSlot = SafeCast<ValueSlot<ValueTypes<T>::Type>*>(slot);
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


void SlotEditor::RebuildSlots() {
  RemoveAllSlots();

  /// The layout object was deleted when mSlotsWidget was removed
  SafeDelete(mSlotsWidget);
  mSlotLayout = nullptr;

  /// Add a widget that stores all slot editors
  mSlotsWidget = new QWidget(mWatcherWidget);
  mLayout->addWidget(mSlotsWidget);
  mSlotsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  QVBoxLayout* mSlotLayout = new QVBoxLayout(mSlotsWidget);
  mSlotLayout->setSpacing(4);
  mSlotLayout->setContentsMargins(0, 0, 0, 0);

  shared_ptr<Node> directNode = GetDirectNode();

  /// Generate slot editors
  for (Slot* slot : directNode->GetPublicSlots()) {
    /// Create horizontal widget to add editor and ghost button
    QWidget* widget = new QWidget(mSlotsWidget);
    mSlotLayout->addWidget(widget);

    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    QHBoxLayout* hLayout = new QHBoxLayout(widget);
    hLayout->setSpacing(4);
    hLayout->setContentsMargins(0, 0, 0, 0);

    if (slot->mIsMultiSlot ||
      (!AddSlot<ValueType::FLOAT>(slot, widget, hLayout) &&
      !AddSlot<ValueType::VEC2>(slot, widget, hLayout) &&
      !AddSlot<ValueType::VEC3>(slot, widget, hLayout) &&
      !AddSlot<ValueType::VEC4>(slot, widget, hLayout))) {
      QLabel* label = new QLabel(QString::fromStdString(slot->mName), widget);
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
    QPushButton* sourceButton = new QPushButton("Edit source", mSlotsWidget);
    mSlotsWidget->connect(sourceButton, &QPushButton::pressed, [=]() {
      ZenGarden::GetInstance()->Watch(
        stub->mSource.GetReferencedNode(), WatcherPosition::RIGHT_TAB);
    });
    mSlotLayout->addWidget(sourceButton);
  }
}

void SlotEditor::RemoveAllSlots() {
  for (auto it : mSlotWatchers) {
    if (it.second->GetDirectNode()) {
      it.second->GetDirectNode()->RemoveWatcher(it.second);
    }
    SafeDelete(it.second->mWatcherWidget);
  }
  mSlotWatchers.clear();
}

void SlotEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);
  RebuildSlots();
}


void SlotEditor::OnSlotConnectionChanged(Slot* slot) {
  RebuildSlots();
}


void SlotEditor::OnSlotStructureChanged() {
  RebuildSlots();
}

void SlotEditor::RemoveWatcherWidget(WatcherWidget* watcherWidget) {
  delete watcherWidget;
}


SlotWatcher::SlotWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node) 
{}


template <ValueType T>
TypedSlotWatcher<T>::TypedSlotWatcher(ValueSlot<Type>* slot)
  : SlotWatcher(slot->GetReferencedNode())
  , mSlot(slot) 
{}


template <ValueType T>
void TypedSlotWatcher<T>::SetWatcherWidget(WatcherWidget* watcherWidget) {
  SlotWatcher::SetWatcherWidget(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(watcherWidget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  shared_ptr<ValueNode<Type>> valueNode = PointerCast<ValueNode<Type>>(GetNode());
  auto value = valueNode->Get();

  mEditor = new ValueEditor<T>(watcherWidget,
    QString::fromStdString(mSlot->mName), value);
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

PassSlotEditor::PassSlotEditor(const shared_ptr<Node>& node)
  : SlotEditor(node)
{}

void PassSlotEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  SlotEditor::SetWatcherWidget(watcherWidget);

  /// Recompile button
  QPushButton* copyVSButton = new QPushButton("Copy VS source", watcherWidget);
  watcherWidget->connect(copyVSButton, &QPushButton::pressed, [=]() {
    shared_ptr<Pass> passNode = PointerCast<Pass>(GetNode());
    string source = passNode->GetVertexShaderSource();
    QApplication::clipboard()->setText(QString::fromStdString(source));
  });
  mLayout->addWidget(copyVSButton);

  QPushButton* copyFSButton = new QPushButton("Copy FS source", watcherWidget);
  watcherWidget->connect(copyFSButton, &QPushButton::pressed, [=]() {
    shared_ptr<Pass> passNode = PointerCast<Pass>(GetNode());
    string source = passNode->GetFragmentShaderSource();
    QApplication::clipboard()->setText(QString::fromStdString(source));
  });
  mLayout->addWidget(copyFSButton);
}
