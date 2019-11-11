#include "sloteditor.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QClipboard>

SlotEditor::SlotEditor(const std::shared_ptr<Node>& node)
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
  if (!IsPointerOf<ValueNode<typename ValueTypes<T>::Type>>(slot->GetReferencedNode())) {
    return false;
  }

  auto valueSlot = SafeCast<ValueSlot<typename ValueTypes<T>::Type>*>(slot);
  auto slotNode = valueSlot->GetReferencedNode();
  std::shared_ptr<SlotWatcher> watcher =
    slotNode->template Watch<TypedSlotWatcher<T>>(valueSlot);

  WatcherWidget* widget =
    new WatcherWidget(parent, watcher, WatcherPosition::PROPERTY_PANEL);
  watcher->mDeleteWatcherWidgetCallback =
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
  // ReSharper disable CppNonReclaimedResourceAcquisition
  QVBoxLayout* slotLayout = new QVBoxLayout(mSlotsWidget);
  // ReSharper restore CppNonReclaimedResourceAcquisition
  slotLayout->setSpacing(4);
  slotLayout->setContentsMargins(0, 0, 0, 0);

  const std::shared_ptr<Node> directNode = GetDirectNode();

  /// Generate slot editors
  for (Slot* slot : directNode->GetPublicSlots()) {
    /// Create horizontal widget to add editor and ghost button
    QWidget* widget = new QWidget(mSlotsWidget);
    slotLayout->addWidget(widget);

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

    QPushButton::connect(ghostButton, &QPushButton::toggled, [=]() {
      slot->SetGhost(!slot->IsGhost());
    });
  }

  /// Source editor button
  if (IsExactType<StubNode>(directNode)) {
    const auto stub = PointerCast<StubNode>(directNode);
    QPushButton* sourceButton = new QPushButton("Edit source", mSlotsWidget);
    QWidget::connect(sourceButton, &QPushButton::pressed, [=]() {
      ZenGarden::GetInstance()->Watch(
        stub->mSource.GetReferencedNode(), WatcherPosition::RIGHT_TAB);
    });
    slotLayout->addWidget(sourceButton);
  }
}

void SlotEditor::RemoveAllSlots() {
  for (const auto& it : mSlotWatchers) {
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

// ReSharper disable once CppMemberFunctionMayBeStatic
void SlotEditor::RemoveWatcherWidget(WatcherWidget* watcherWidget) {
  delete watcherWidget;
}


SlotWatcher::SlotWatcher(const std::shared_ptr<Node>& node)
  : WatcherUi(node) 
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

  std::shared_ptr<ValueNode<Type>> valueNode = PointerCast<ValueNode<Type>>(GetNode());
  auto value = valueNode->Get();

  mEditor = new ValueEditor<T>(watcherWidget,
    QString::fromStdString(mSlot->mName), value);
  mEditor->onValueChange += Delegate(this, &TypedSlotWatcher<T>::HandleValueChange);

  layout->addWidget(mEditor);

  vec2 range = mSlot->GetRange();
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

PassSlotEditor::PassSlotEditor(const std::shared_ptr<Node>& node)
  : SlotEditor(node)
{}

void PassSlotEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  SlotEditor::SetWatcherWidget(watcherWidget);

  /// Recompile button
  QPushButton* copyVsButton = new QPushButton("Copy VS source", watcherWidget);
  WatcherWidget::connect(copyVsButton, &QPushButton::pressed, [=]() {
    const std::shared_ptr<Pass> passNode = PointerCast<Pass>(GetNode());
    const std::string source = passNode->GetVertexShaderSource();
    QApplication::clipboard()->setText(QString::fromStdString(source));
  });
  mLayout->addWidget(copyVsButton);

  QPushButton* copyFsButton = new QPushButton("Copy FS source", watcherWidget);
  WatcherWidget::connect(copyFsButton, &QPushButton::pressed, [=]() {
    const std::shared_ptr<Pass> passNode = PointerCast<Pass>(GetNode());
    const std::string source = passNode->GetFragmentShaderSource();
    QApplication::clipboard()->setText(QString::fromStdString(source));
  });
  mLayout->addWidget(copyFsButton);
}
