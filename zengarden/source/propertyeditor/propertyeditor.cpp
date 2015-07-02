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
	QLabel* typeLabel = new QLabel(ThePrototypes->GetNodeClassString(node), panel);
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
	QLabel* nameLabel = new QLabel("Name:", nameEditor);
	nameEditorLayout->addWidget(nameLabel);
	QLineEdit* nameLineEdit = new QLineEdit(nameEditor);
	nameEditorLayout->addWidget(nameLineEdit);
	//nameLineEdit->setText(QString::fromStdString(node->mName));
	mLayout->addWidget(nameEditor);
}


DefaultPropertyEditor::DefaultPropertyEditor(Node* node, WatcherWidget* panel)
  : PropertyEditor(node, panel)
{
    /// Slots
    for (Slot* slot : GetNode()->mSlots) {
      if (slot->DoesAcceptType(NodeType::FLOAT) && 
          slot->GetNode()->GetType() != NodeType::SHADER_STUB) {
        /// Float slots
        WatcherWidget* widget =
          new WatcherWidget(panel, WatcherPosition::PROPERTY_PANEL);
        FloatWatcher* watcher = new FloatWatcher(
          static_cast<ValueNode<NodeType::FLOAT>*>(slot->GetNode()), widget,
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

void DefaultPropertyEditor::HandleSniffedMessage(Slot* slot, NodeMessage message, 
                                                 const void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    {
      auto it = mSlotWatchers.find(slot);
      if (it != mSlotWatchers.end()) {
        Watcher* watcher = it->second;
        if (slot->GetNode()->GetType() == NodeType::SHADER_STUB) {
          watcher->ChangeNode(nullptr);
        } else {
          watcher->ChangeNode(slot->GetNode());
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
  new FloatWatcher(static_cast<FloatNode*>(GetNode()), widget, valueString);
  mLayout->addWidget(widget);
}

