#include "documentwatcher.h"
#include "../zengarden.h"

Q_DECLARE_METATYPE(Graph*)

enum MyRoles {
  GraphNodeRole = Qt::UserRole + 1
};


DocumentWatcher::DocumentWatcher(Document* document)
  : WatcherUI(document)
{
}


DocumentWatcher::~DocumentWatcher() {
  SafeDelete(mModel);
}


void DocumentWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  mUI.setupUi(watcherWidget);

  mModel = new QStandardItemModel();
  mUI.graphList->setModel(mModel);

  watcherWidget->connect(mUI.graphList, &QListView::clicked, [=](const QModelIndex &index) {
    QStandardItem* item = this->mModel->itemFromIndex(index);
    Graph* graph = item->data().value<Graph*>();
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(graph);
  });

  RefreshGraphList();

  watcherWidget->connect(mUI.openButton, &QPushButton::pressed, [=]() { 
    QModelIndex index = mUI.graphList->currentIndex();
    if (!index.isValid()) return;
    QStandardItem* item = this->mModel->itemFromIndex(index);
    Graph* graph = item->data().value<Graph*>();
    ZenGarden::GetInstance()->Watch(graph, WatcherPosition::RIGHT_TAB);
  });

  watcherWidget->connect(mUI.newGraphButton, &QPushButton::pressed, [=]() {
    Graph* graph = new Graph();
    Document* document = dynamic_cast<Document*>(this->mNode);
    document->mGraphs.Connect(graph);
  });

  //watcherWidget->connect(mUI.removePointButton, &QPushButton::pressed, [=]() { RemovePoint(); });
  //watcherWidget->connect(mUI.linearCheckBox, &QPushButton::pressed, [=]() { ToggleLinear(); });
}

void DocumentWatcher::OnSlotConnectionChanged(Slot* slot) {
  RefreshGraphList();
}

void DocumentWatcher::OnChildNameChange() {
  RefreshGraphList();
}

void DocumentWatcher::RefreshGraphList() {
  Document* doc = static_cast<Document*>(mNode);
  mModel->clear();
  for (Node* node : doc->mGraphs.GetMultiNodes()) {
    Graph* graph = static_cast<Graph*>(node);
    QStandardItem* item = new QStandardItem(CreateDisplayedName(graph));
    item->setData(QVariant::fromValue(graph), GraphNodeRole);
    mModel->appendRow(item);
    //GraphNode* p = index.data(GraphNodeRole).value<GraphNode*>();
  }
}


