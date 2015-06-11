#include "propertyeditor.h"
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
	nameLineEdit->setText(QString::fromStdString(node->mName));
	mLayout->addWidget(nameEditor);
}

PropertyEditor::~PropertyEditor()
{}

//void PropertyEditor::BuildPanel() {
//  /// Slots
//  for (Slot* slot : GetNode()->mSlots) {
//    QLabel* slotLabel = 
//      new QLabel(QString::fromStdString(*slot->GetName()), mWatcherWidget);
//    mLayout->addWidget(slotLabel);
//  }
//}
