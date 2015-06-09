#include "propertyeditor.h"
#include "../graph/prototypes.h"
#include "staticvalueeditor.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>

PropertyEditor::PropertyEditor(Node* Nd, QWidget* _PropertyPanel)
	: Node(NodeType::UI, "PropertyEditor")
	, WatchedNode(NodeType::ALLOW_ALL, this, nullptr)
	, PropertyPanel(_PropertyPanel)
{
	WatchedNode.Connect(Nd);

	/// Vertical layout
	Layout = new QVBoxLayout(PropertyPanel);
	//Layout->setSpacing(6);
	//Layout->setContentsMargins(0, 0, 0, 0);

	/// Node type
	QLabel* typeLabel = new QLabel(ThePrototypes->GetNodeClassString(Nd), PropertyPanel);
	typeLabel->setAlignment(Qt::AlignHCenter);
	QFont font = typeLabel->font();
	font.setBold(true);
	typeLabel->setFont(font);
	Layout->addWidget(typeLabel);

	/// Name input
	QWidget* nameEditor = new QWidget(PropertyPanel);
	QHBoxLayout* nameEditorLayout = new QHBoxLayout(nameEditor);
	nameEditorLayout->setSpacing(6);
	nameEditorLayout->setContentsMargins(0, 0, 0, 0);
	QLabel* nameLabel = new QLabel("Name:", nameEditor);
	nameEditorLayout->addWidget(nameLabel);
	QLineEdit* nameLineEdit = new QLineEdit(nameEditor);
	nameEditorLayout->addWidget(nameLineEdit);
	nameLineEdit->setText(QString::fromStdString(Nd->name));
	Layout->addWidget(nameEditor);

	/// Slots
	for (Slot* slot : Nd->slotList)
	{
		QLabel* slotLabel = new QLabel(QString::fromStdString(*slot->GetName()), PropertyPanel);
		Layout->addWidget(slotLabel);
	}

	/// Spacer on bottom
	//QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
	//Layout->addItem(spacer);
}

PropertyEditor::~PropertyEditor()
{
	while (QWidget* w = PropertyPanel->findChild<QWidget*>())
	{
		delete w;
	}
	delete Layout;
}

PropertyEditor* PropertyEditor::ForNode(Node* Nd, QWidget* PropertyPanel)
{
	switch (ThePrototypes->GetNodeClass(Nd))
	{
	case NodeClass::STATIC_FLOAT:
		return new StaticFloatEditor(Nd, PropertyPanel);
	default:
		return new PropertyEditor(Nd, PropertyPanel);
	}

}

