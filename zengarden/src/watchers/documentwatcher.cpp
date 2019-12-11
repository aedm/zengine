#include "documentwatcher.h"
#include "../zengarden.h"
#include <memory>

Q_DECLARE_METATYPE(std::shared_ptr<Graph>)

enum MyRoles {
  GRAPH_NODE_ROLE = Qt::UserRole + 1
};


DocumentWatcher::DocumentWatcher(const std::shared_ptr<Node>& document)
  : WatcherUi(document)
{}


DocumentWatcher::~DocumentWatcher() {
  SafeDelete(mModel);
}


void DocumentWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);
  mUi.setupUi(watcherWidget);

  mUi.graphList->setFocusPolicy(Qt::NoFocus);
  mUi.openButton->setFocusPolicy(Qt::NoFocus);
  mUi.newGraphButton->setFocusPolicy(Qt::NoFocus);
  mUi.deleteGraphButton->setFocusPolicy(Qt::NoFocus);

  mModel = new QStandardItemModel();
  mUi.graphList->setModel(mModel);

  QObject::connect(mUi.graphList, &QListView::clicked,
    [=](const QModelIndex &index) {
    QStandardItem* item = this->mModel->itemFromIndex(index);
    const std::shared_ptr<Graph> graph = item->data().value<std::shared_ptr<Graph>>();
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(graph);
  });

  RefreshGraphList();

  QObject::connect(mUi.openButton, &QPushButton::pressed, [=]() {
    const QModelIndex index = mUi.graphList->currentIndex();
    if (!index.isValid()) return;
    QStandardItem* item = this->mModel->itemFromIndex(index);
    const std::shared_ptr<Graph> graph = item->data().value<std::shared_ptr<Graph>>();
    ZenGarden::GetInstance()->Watch(graph, WatcherPosition::RIGHT_TAB);
  });

  QObject::connect(mUi.newGraphButton, &QPushButton::pressed, [=]() {
    const std::shared_ptr<Graph> graph = std::make_shared<Graph>();
    std::shared_ptr<Document> document = PointerCast<Document>(GetNode());
    document->mGraphs.Connect(graph);
  });
}

void DocumentWatcher::OnSlotConnectionChanged(Slot* slot) {
  RefreshGraphList();
}

void DocumentWatcher::OnChildNameChange() {
  RefreshGraphList();
}

void DocumentWatcher::RefreshGraphList() const
{
  const std::shared_ptr<Document> doc = PointerCast<Document>(GetNode());
  mModel->clear();
  for (const auto& node : doc->mGraphs.GetDirectMultiNodes()) {
    std::shared_ptr<Graph> graph = PointerCast<Graph>(node);
    QStandardItem* item = new QStandardItem(CreateDisplayedName(graph));
    item->setData(QVariant::fromValue(graph), GRAPH_NODE_ROLE);
    mModel->appendRow(item);
  }
}


