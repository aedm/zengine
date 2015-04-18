#include "operatorprototypes.h"
#include <ui_operatorSelector.h>

OperatorPrototypes* ThePrototypes = NULL;

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


OperatorPrototypes::OperatorPrototypes()
{
	Prototypes.push_back(new FloatNode());
	Prototypes.push_back(new Vec4Node());
	Prototypes.push_back(new TextureNode());
}

OperatorPrototypes::~OperatorPrototypes()
{
	foreach (Node* nd, Prototypes) 
	{
		delete nd;
	}
	Prototypes.clear();
}

Node* OperatorPrototypes::AskUser(QPoint Position)
{
	QDialog dialog(NULL, Qt::FramelessWindowHint);
	Dialog = &dialog;
	Ui::OperatorSelector selector;
	selector.setupUi(&dialog);
	for (int i=0; i<Prototypes.size(); i++)
	{
		selector.treeWidget->addTopLevelItem(
			new SelectorItem(NULL, QString::fromStdString(Prototypes[i]->Name), i+1));
	}
	dialog.setModal(true);
	dialog.resize(150, 300);
	dialog.move(Position);
	connect(selector.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemSelected(QTreeWidgetItem*, int)));
	//dialog.connect(SIGNAL(itemClicked()), this, SLOT(OnItemSelected()));
	int ret = dialog.exec();
	return (ret > 0) ? Prototypes[ret-1]->Clone() : NULL;
}

void OperatorPrototypes::OnItemSelected(QTreeWidgetItem* Item, int)
{
	SelectorItem* item = static_cast<SelectorItem*>(Item);
	if (item->NodeIndex >= 0) Dialog->done(item->NodeIndex);
}

void OperatorPrototypes::Init()
{
	ThePrototypes = new OperatorPrototypes();
}

void OperatorPrototypes::Dispose()
{
	SafeDelete(ThePrototypes);
}
