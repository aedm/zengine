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
  mNode->SetName(mNameTextBox->text().toStdString());
}


DefaultPropertyEditor::DefaultPropertyEditor(Node* node, WatcherWidget* panel)
  : PropertyEditor(node, panel)
{
  /// Slots
  for (Slot* slot : mNode->GetPublicSlots()) {
    /// TODO: use dynamic_cast
    if (slot->DoesAcceptType(NodeType::FLOAT) && 
        slot->GetAbstractNode()->GetType() != NodeType::SHADER_STUB) {
      /// Float slots
      WatcherWidget* widget =
        new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
      FloatWatcher* watcher = new FloatWatcher(
        static_cast<FloatNode*>(slot->GetAbstractNode()), widget,
        QString::fromStdString(*slot->GetName()));

      if (!static_cast<FloatSlot*>(slot)->IsDefaulted()) {
        watcher->SetReadOnly(true);
      }
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
            bool defaulted = static_cast<FloatSlot*>(slot)->IsDefaulted();
            static_cast<FloatWatcher*>(watcher)->SetReadOnly(!defaulted);
          }
        }
      }
      break;
    }
    default: break;
  }
}


StaticFloatEditor::StaticFloatEditor(FloatNode* node, WatcherWidget* panel)
  : PropertyEditor(node, panel) {
  static const QString valueString("value");
  WatcherWidget* widget = new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
  new FloatWatcher(static_cast<FloatNode*>(mNode), widget, valueString);
  mLayout->addWidget(widget);
}

