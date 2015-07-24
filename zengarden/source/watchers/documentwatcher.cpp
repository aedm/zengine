#include "documentwatcher.h"

Q_DECLARE_METATYPE(Graph*)

enum MyRoles {
  GraphNodeRole = Qt::UserRole + 1
};


DocumentWatcher::DocumentWatcher(QListView* listView, Document* document)
  : Watcher(document, nullptr)
  , mListView(listView) {
  mModel = new QStandardItemModel();
  mListView->setModel(mModel);
}


DocumentWatcher::~DocumentWatcher() {
  delete mModel;
}


void DocumentWatcher::HandleSniffedMessage(Slot* slot, NodeMessage message, 
                                           void* payload) {
  Document* doc = static_cast<Document*>(mNode);
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    {
      mModel->clear();
      for (Node* node : doc->mGraphs.GetMultiNodes()) {
        Graph* graph = static_cast<Graph*>(node);
        QStandardItem* item = new QStandardItem(QString("graph"));
        item->setData(QVariant::fromValue(graph), GraphNodeRole);
        mModel->appendRow(item);
        //GraphNode* p = index.data(GraphNodeRole).value<GraphNode*>();
      }
      break;
    }
    default:
      break;
  }
}


