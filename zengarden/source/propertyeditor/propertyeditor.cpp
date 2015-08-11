#include "propertyeditor.h"
#include "parameterwidgets.h"
#include "../graph/prototypes.h"
#include "../watchers/watcherwidget.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>

PropertyEditor::PropertyEditor(Node* node, WatcherWidget* panel)
	: Watcher(node, panel, NodeType::UI)
{
	/// Vertical layout
	mLayout = new QVBoxLayout(panel);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

	/// Node type
  string& typeString = NodeRegistry::GetInstance()->GetNodeClass(node)->mClassName;
  QLabel* typeLabel = new QLabel(QString::fromStdString(typeString), panel);
	typeLabel->setAlignment(Qt::AlignHCenter);
	QFont font = typeLabel->font();
	font.setBold(true);
	typeLabel->setFont(font);
	mLayout->addWidget(typeLabel);

	/// Name input
	QWidget* nameEditor = new QWidget(panel);
	QHBoxLayout* nameEditorLayout = new QHBoxLayout(nameEditor);
	nameEditorLayout->setSpacing(6);
	nameEditorLayout->setContentsMargins(0, 0, 0, 0);
	//QLabel* nameLabel = new QLabel("Name:", nameEditor);
	//nameEditorLayout->addWidget(nameLabel);
	mNameTextBox = new TextBox(nameEditor);
  mNameTextBox->setPlaceholderText(QString("noname"));
  if (!node->GetName().empty()) {
    mNameTextBox->setText(QString::fromStdString(node->GetName()));
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


DefaultPropertyEditor::DefaultPropertyEditor(Node* node, WatcherWidget* panel)
  : PropertyEditor(node, panel)
{
  /// Slots
  for (Slot* slot : mNode->GetPublicSlots()) {
    WatcherWidget* widget = nullptr;
    Watcher* watcher = nullptr;

    /// TODO: use dynamic_cast
    if (slot->DoesAcceptType(NodeType::FLOAT) && 
        slot->GetAbstractNode()->GetType() != NodeType::SHADER_STUB) {
      /// Float slots
      widget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
      ASSERT(dynamic_cast<ValueNode<NodeType::FLOAT>*>(slot->GetAbstractNode()));
      watcher = new FloatWatcher(
        static_cast<ValueNode<NodeType::FLOAT>*>(slot->GetAbstractNode()),
        widget, QString::fromStdString(*slot->GetName()), !slot->IsDefaulted());
    } 
    else if (slot->DoesAcceptType(NodeType::VEC3) &&
             slot->GetAbstractNode()->GetType() != NodeType::SHADER_STUB) {
      /// Vec3 slots
      widget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
      ASSERT(dynamic_cast<ValueNode<NodeType::VEC3>*>(slot->GetAbstractNode()));
      watcher = new Vec3Watcher(
        static_cast<ValueNode<NodeType::VEC3>*>(slot->GetAbstractNode()),
        widget, QString::fromStdString(*slot->GetName()), !slot->IsDefaulted());
    }
    else if (slot->DoesAcceptType(NodeType::VEC4) &&
             slot->GetAbstractNode()->GetType() != NodeType::SHADER_STUB) {
      /// Vec4 slots
      widget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
      ASSERT(dynamic_cast<ValueNode<NodeType::VEC4>*>(slot->GetAbstractNode()));
      watcher = new Vec4Watcher(
        static_cast<ValueNode<NodeType::VEC4>*>(slot->GetAbstractNode()),
        widget, QString::fromStdString(*slot->GetName()), !slot->IsDefaulted());
    }

    if (watcher != nullptr) {
      widget->onWatcherDeath =
        Delegate(this, &DefaultPropertyEditor::RemoveWatcherWidget);
      mLayout->addWidget(widget);
      mSlotWatchers[slot] = watcher;
    } else {
      /// General slots, just add a label
      QLabel* slotLabel =
        new QLabel(QString::fromStdString(*slot->GetName()), mWatcherWidget);
      mLayout->addWidget(slotLabel);
    }
  }
}

void DefaultPropertyEditor::HandleSniffedMessage(NodeMessage message, Slot* slot,
                                                 void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    {
      auto it = mSlotWatchers.find(slot);
      if (it != mSlotWatchers.end()) {
        Watcher* watcher = it->second;
        if (slot->GetAbstractNode()->GetType() == NodeType::SHADER_STUB) {
          watcher->ChangeNode(nullptr);
        } else {
          watcher->ChangeNode(slot->GetAbstractNode());
          if (slot->DoesAcceptType(NodeType::FLOAT)) {
            ASSERT(dynamic_cast<FloatWatcher*>(watcher));
            static_cast<FloatWatcher*>(watcher)->SetReadOnly(!slot->IsDefaulted());
          }
          else if (slot->DoesAcceptType(NodeType::VEC3)) {
            ASSERT(dynamic_cast<Vec3Watcher*>(watcher));
            static_cast<Vec3Watcher*>(watcher)->SetReadOnly(!slot->IsDefaulted());
          }
          else if (slot->DoesAcceptType(NodeType::VEC4)) {
            ASSERT(dynamic_cast<Vec4Watcher*>(watcher));
            static_cast<Vec4Watcher*>(watcher)->SetReadOnly(!slot->IsDefaulted());
          }
        }
      }
      break;
    }
    default: break;
  }
}

void DefaultPropertyEditor::RemoveWatcherWidget(WatcherWidget* watcherWidget) {
  delete watcherWidget;
}


StaticFloatEditor::StaticFloatEditor(FloatNode* node, WatcherWidget* panel)
  : PropertyEditor(node, panel) 
{
  static const QString valueString("value");
  mValueWatcherWidget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
  new FloatWatcher(
    static_cast<FloatNode*>(mNode), mValueWatcherWidget, valueString, false);
  mLayout->addWidget(mValueWatcherWidget);
  mValueWatcherWidget->onWatcherDeath = 
    Delegate(this, &StaticFloatEditor::RemoveStaticWatcher);
}

void StaticFloatEditor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}


StaticVec3Editor::StaticVec3Editor(Vec3Node* node, WatcherWidget* panel)
  : PropertyEditor(node, panel)
{
  static const QString valueString("value");
  mValueWatcherWidget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
  new Vec3Watcher(
    static_cast<Vec3Node*>(mNode), mValueWatcherWidget, valueString, false);
  mLayout->addWidget(mValueWatcherWidget);
  mValueWatcherWidget->onWatcherDeath =
    Delegate(this, &StaticVec3Editor::RemoveStaticWatcher);
}

void StaticVec3Editor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}




StaticVec4Editor::StaticVec4Editor(Vec4Node* node, WatcherWidget* panel)
  : PropertyEditor(node, panel) 
{
  static const QString valueString("value");
  mValueWatcherWidget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
  new Vec4Watcher(
    static_cast<Vec4Node*>(mNode), mValueWatcherWidget, valueString, false);
  mLayout->addWidget(mValueWatcherWidget);
  mValueWatcherWidget->onWatcherDeath = 
    Delegate(this, &StaticVec4Editor::RemoveStaticWatcher);
}

void StaticVec4Editor::RemoveStaticWatcher(WatcherWidget* watcherWidget) {
  ASSERT(mValueWatcherWidget == watcherWidget);
  SafeDelete(mValueWatcherWidget);
}
