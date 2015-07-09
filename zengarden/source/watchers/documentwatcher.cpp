#include "documentwatcher.h"

Q_DECLARE_METATYPE(Graph*)

enum MyRoles {
	GraphNodeRole = Qt::UserRole + 1
};


DocumentWatcher::DocumentWatcher(QListView* _ListView, Document* DocumentNode)
	: Watcher(DocumentNode, nullptr)
	, ListView(_ListView)
{
	Model = new QStandardItemModel();
	ListView->setModel(Model);
}


DocumentWatcher::~DocumentWatcher()
{
	delete Model;
}


void DocumentWatcher::HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	Document* doc = static_cast<Document*>(mWatchedNode.GetAbstractNode());
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
	{
		Model->clear();
		for (Node* node : doc->mGraphs.GetMultiNodes())
		{
			Graph* graph = static_cast<Graph*>(node);
			QStandardItem* item = new QStandardItem(QString("graph"));
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


