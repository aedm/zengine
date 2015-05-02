#include "documentwatcher.h"

Q_DECLARE_METATYPE(GraphNode*)

enum MyRoles {
	GraphNodeRole = Qt::UserRole + 1
};

DocumentWatcher::DocumentWatcher(QListView* _ListView, Document* DocumentNode)
	: Node(NodeType::DOCUMENT, "DocumentWatcher")
	, DocumentSlot(NodeType::DOCUMENT, this, nullptr)
	, ListView(_ListView)
{
	DocumentSlot.Connect(DocumentNode);
	Model = new QStandardItemModel();
	ListView->setModel(Model);
}

void DocumentWatcher::HandleMessage(Slot* S, NodeMessage Message)
{
	switch (Message)
	{
	case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
	{
		Document* doc = static_cast<Document*>(DocumentSlot.GetNode());
		Model->clear();
		for (Node* node : doc->Graphs.GetMultiNodes())
		{
			GraphNode* graph = static_cast<GraphNode*>(node);
			QStandardItem* item = new QStandardItem(QString::fromStdString(graph->Name));
			item->setData(QVariant::fromValue(graph), GraphNodeRole);
			Model->appendRow(item);
			//GraphNode* p = index.data(GraphNodeRole).value<GraphNode*>();
		}
		break;
	}
	default:
		break;
	}
}

DocumentWatcher::~DocumentWatcher()
{
	delete Model;
}

