#include "propertyeditor.h"
#include "parameterwidgets.h"
#include "../graph/prototypes.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QPushButton>

PropertyEditor::PropertyEditor(Node* node)
  : WatcherUI(node) {}


void PropertyEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  /// Node type
  string& typeString = NodeRegistry::GetInstance()->GetNodeClass(mNode)->mClassName;
  QLabel* typeLabel = new QLabel(QString::fromStdString(typeString), watcherWidget);
  typeLabel->setAlignment(Qt::AlignHCenter);
  QFont font = typeLabel->font();
  font.setBold(true);
  typeLabel->setFont(font);
  mLayout->addWidget(typeLabel);

  /// Name input
  QWidget* nameEditor = new QWidget(watcherWidget);
  QHBoxLayout* nameEditorLayout = new QHBoxLayout(nameEditor);
  nameEditorLayout->setSpacing(6);
  nameEditorLayout->setContentsMargins(0, 0, 0, 0);
  mNameTextBox = new TextBox(nameEditor);
  mNameTextBox->setPlaceholderText(QString("noname"));
  if (!mNode->GetName().empty()) {
    mNameTextBox->setText(QString::fromStdString(mNode->GetName()));
  }
  mNameTextBox->onEditingFinished +=
    Delegate(this, &PropertyEditor::HandleNameTexBoxChanged);
  nameEditorLayout->addWidget(mNameTextBox);
  mLayout->addWidget(nameEditor);
}

void PropertyEditor::HandleNameTexBoxChanged() {
  if (mNode) {
    mNode->SetName(mNameTextBox->text().toStdString());
  }
}


DefaultPropertyEditor::DefaultPropertyEditor(Node* node)
  : PropertyEditor(node) {}


DefaultPropertyEditor::~DefaultPropertyEditor() {
  for (auto it : mSlotWatchers) {
    if (it.second->GetNode()) it.second->GetNode()->RemoveWatcher(it.second.get());
    SafeDelete(it.second->mWatcherWidget);
  }
}

void DefaultPropertyEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  /// Slots
  for (Slot* slot : mNode->GetPublicSlots()) {
    if (slot->mIsMultiSlot) continue;

    shared_ptr<WatcherUI> watcher;

    if (IsInsanceOf<ValueNode<ValueType::FLOAT>*>(slot->GetReferencedNode())) {
      /// Float slots
      auto floatSlot = SafeCast<FloatSlot*>(slot);
      auto slotNode = floatSlot->GetNode();
      watcher = slotNode->Watch<FloatWatcher>(slotNode, floatSlot,
        QString::fromStdString(*slot->GetName()),
        !slot->IsDefaulted());
    }
    else if (IsInsanceOf<ValueNode<ValueType::VEC3>*>(slot->GetReferencedNode())) {
      /// Vec3 slots
      auto vec3Slot = SafeCast<Vec3Slot*>(slot);
      auto slotNode = vec3Slot->GetNode();
      watcher = slotNode->Watch<Vec3Watcher>(slotNode, vec3Slot,
        QString::fromStdString(*slot->GetName()),
        !slot->IsDefaulted());
    }
    else if (IsInsanceOf<ValueNode<ValueType::VEC4>*>(slot->GetReferencedNode())) {
      /// Vec4 slots
      auto vec4Slot = SafeCast<Vec4Slot*>(slot);
      auto slotNode = vec4Slot->GetNode();
      watcher = slotNode->Watch<Vec4Watcher>(
        slotNode, QString::fromStdString(*slot->GetName()), !slot->IsDefaulted());
    }

    if (watcher) {
      WatcherWidget* widget = new WatcherWidget(watcherWidget, watcher, WatcherPosition::PROPERTY_PANEL);
      watcher->deleteWatcherWidgetCallback = Delegate(this, &DefaultPropertyEditor::RemoveWatcherWidget);
      mLayout->addWidget(widget);
      watcher->SetWatcherWidget(widget);
      mSlotWatchers[slot] = watcher;
    }
    else {
      /// General slots, just add a label
      //QLabel* slotLabel =
      //  new QLabel(QString::fromStdString(*slot->GetName()), mWatcherWidget);
      //mLayout->addWidget(slotLabel);
    }
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

void DefaultPropertyEditor::OnSlotConnectionChanged(Slot* slot) {
  auto it = mSlotWatchers.find(slot);
  if (it != mSlotWatchers.end()) {
    shared_ptr<WatcherUI> watcher = it->second;
    Node* node = slot->GetReferencedNode();
    if (IsInsanceOf<StubNode*>(node)) {
      node->RemoveWatcher(watcher.get());
    }
    else {
      slot->GetReferencedNode()->AssignWatcher(watcher);
      WatcherUI* watcherPtr = watcher.get();
      if (slot->DoesAcceptValueNode(ValueType::FLOAT)) {
        SafeCast<FloatWatcher*>(watcherPtr)->SetReadOnly(!slot->IsDefaulted());
      }
      else if (slot->DoesAcceptValueNode(ValueType::VEC3)) {
        SafeCast<Vec3Watcher*>(watcherPtr)->SetReadOnly(!slot->IsDefaulted());
      }
      else if (slot->DoesAcceptValueNode(ValueType::VEC4)) {
        SafeCast<Vec4Watcher*>(watcherPtr)->SetReadOnly(!slot->IsDefaulted());
      }
    }
  }
}


void DefaultPropertyEditor::RemoveWatcherWidget(WatcherWidget* watcherWidget) {
  delete watcherWidget;
}


StaticFloatEditor::StaticFloatEditor(FloatNode* node)
  : PropertyEditor(node) {}


void StaticFloatEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  static const QString valueString("value");
  auto watcher = mNode->Watch<FloatWatcher>(static_cast<FloatNode*>(mNode), nullptr,
    valueString, false);
  mValueWatcherWidget = new WatcherWidget(watcherWidget, watcher,
    WatcherPosition::PROPERTY_PANEL);
  mLayout->addWidget(mValueWatcherWidget);
}

void StaticFloatEditor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}


StaticVec3Editor::StaticVec3Editor(Vec3Node* node)
  : PropertyEditor(node) {}


void StaticVec3Editor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  static const QString valueString("value");
  auto watcher = mNode->Watch<Vec3Watcher>(static_cast<Vec3Node*>(mNode), nullptr,
    valueString, false);
  mValueWatcherWidget = new WatcherWidget(watcherWidget, watcher,
    WatcherPosition::PROPERTY_PANEL);
  mLayout->addWidget(mValueWatcherWidget);
}

void StaticVec3Editor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}


StaticVec4Editor::StaticVec4Editor(Vec4Node* node)
  : PropertyEditor(node) {}


void StaticVec4Editor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  static const QString valueString("value");
  auto watcher = mNode->Watch<Vec4Watcher>(static_cast<Vec4Node*>(mNode), valueString, false);
  mValueWatcherWidget = new WatcherWidget(watcherWidget, watcher, WatcherPosition::PROPERTY_PANEL);
  mLayout->addWidget(mValueWatcherWidget);
  //mValueWatcherWidget->onWatcherDeath =
  //  Delegate(this, &StaticVec4Editor::RemoveStaticWatcher);
}

void StaticVec4Editor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}
