#include "documentwatcher.h"
#include "../zengarden.h"

Q_DECLARE_METATYPE(shared_ptr<Graph>)

enum MyRoles {
  GraphNodeRole = Qt::UserRole + 1
};


DocumentWatcher::DocumentWatcher(const shared_ptr<Node>& document)
  : WatcherUI(document)
{}


DocumentWatcher::~DocumentWatcher() {
  SafeDelete(mModel);
}


void DocumentWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  mUI.setupUi(watcherWidget);

  mUI.graphList->setFocusPolicy(Qt::NoFocus);
  mUI.openButton->setFocusPolicy(Qt::NoFocus);
  mUI.newGraphButton->setFocusPolicy(Qt::NoFocus);
  mUI.deleteGraphButton->setFocusPolicy(Qt::NoFocus);

  mModel = new QStandardItemModel();
  mUI.graphList->setModel(mModel);

  watcherWidget->connect(mUI.graphList, &QListView::clicked,
    [=](const QModelIndex &index) {
    QStandardItem* item = this->mModel->itemFromIndex(index);
    shared_ptr<Graph> graph = item->data().value<shared_ptr<Graph>>();
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(graph);
  });

  RefreshGraphList();

  watcherWidget->connect(mUI.openButton, &QPushButton::pressed, [=]() {
    QModelIndex index = mUI.graphList->currentIndex();
    if (!index.isValid()) return;
    QStandardItem* item = this->mModel->itemFromIndex(index);
    shared_ptr<Graph> graph = item->data().value<shared_ptr<Graph>>();
    ZenGarden::GetInstance()->Watch(graph, WatcherPosition::RIGHT_TAB);
  });

  watcherWidget->connect(mUI.newGraphButton, &QPushButton::pressed, [=]() {
    shared_ptr<Graph> graph = make_shared<Graph>();
    shared_ptr<Document> document = PointerCast<Document>(GetNode());
    document->mGraphs.Connect(graph);
  });
}

void DocumentWatcher::OnSlotConnectionChanged(Slot* slot) {
  RefreshGraphList();
}

void DocumentWatcher::OnChildNameChange() {
  RefreshGraphList();
}

void DocumentWatcher::RefreshGraphList() {
  shared_ptr<Document> doc = PointerCast<Document>(GetNode());
  mModel->clear();
  for (const auto& node : doc->mGraphs.GetDirectMultiNodes()) {
    shared_ptr<Graph> graph = PointerCast<Graph>(node);
    QStandardItem* item = new QStandardItem(CreateDisplayedName(graph));
    item->setData(QVariant::fromValue(graph), GraphNodeRole);
    mModel->appendRow(item);
  }
}


