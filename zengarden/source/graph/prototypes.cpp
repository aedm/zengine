#include "prototypes.h"
#include <ui_operatorSelector.h>

Prototypes* ThePrototypes = NULL;

class SelectorItem: public QTreeWidgetItem 
{
public:
	SelectorItem(SelectorItem* Parent, QString Label, int OpIndex)
		: QTreeWidgetItem(Parent)
	{
		setText(0, Label);
		this->NodeIndex = OpIndex;
	}

	int NodeIndex;
};


Prototypes::Prototypes()
{
	AddPrototype(new FloatNode(),		NodeClass::STATIC_FLOAT);
	AddPrototype(new Vec4Node(),		NodeClass::STATIC_VEC4);
	AddPrototype(new TextureNode(),		NodeClass::STATIC_TEXTURE);
}

void Prototypes::AddPrototype(Node* node, NodeClass nodeClass)
{
	PrototypeNodes.push_back(node);
	NodeIndexMap[type_index(typeid(*node))] = nodeClass;
}

Prototypes::~Prototypes()
{
	foreach(Node* nd, PrototypeNodes)
	{
		delete nd;
	}
	PrototypeNodes.clear();
}

Node* Prototypes::AskUser(QWidget* Parent, QPoint Position)
{
	QDialog dialog(Parent, Qt::FramelessWindowHint);
	Dialog = &dialog;
	Ui::OperatorSelector selector;
	selector.setupUi(&dialog);
	for (int i = 0; i<PrototypeNodes.size(); i++)
	{
		selector.treeWidget->addTopLevelItem(
			new SelectorItem(NULL, QString::fromStdString(PrototypeNodes[i]->Name), i + 1));
	}
	dialog.setModal(true);
	dialog.resize(150, 300);
	dialog.move(Position);
	connect(selector.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemSelected(QTreeWidgetItem*, int)));
	//dialog.connect(SIGNAL(itemClicked()), this, SLOT(OnItemSelected()));
	int ret = dialog.exec();
	return (ret > 0) ? PrototypeNodes[ret-1]->Clone() : NULL;
}

void Prototypes::OnItemSelected(QTreeWidgetItem* Item, int)
{
	SelectorItem* item = static_cast<SelectorItem*>(Item);
	if (item->NodeIndex >= 0) Dialog->done(item->NodeIndex);
}

void Prototypes::Init()
{
	ThePrototypes = new Prototypes();
}

void Prototypes::Dispose()
{
	SafeDelete(ThePrototypes);
}
