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
    //delete mUI.graphList->widget(index);
    QStandardItem* item = this->mModel->itemFromIndex(index);
    Graph* graph = item->data().value<Graph*>();
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(graph);
  });


  RefreshGraphList();

  //QVBoxLayout* layout = new QVBoxLayout(mUI.splineFrame);
  //layout->setContentsMargins(0, 0, 0, 0);

  //mSplineWidget = new SplineWidget(mUI.splineFrame);
  //layout->addWidget(mSplineWidget);
  //mSplineWidget->mOnPaint += Delegate(this, &SplineWatcher<T>::DrawSpline);
  //mSplineWidget->OnMousePress += Delegate(this, &SplineWatcher<T>::HandleMouseDown);
  //mSplineWidget->OnMouseRelease += Delegate(this, &SplineWatcher<T>::HandleMouseUp);
  //mSplineWidget->OnMouseMove += Delegate(this, &SplineWatcher<T>::HandleMouseMove);
  //mSplineWidget->OnMouseWheel += Delegate(this, &SplineWatcher<T>::HandleMouseWheel);

  //UpdateRangeLabels();
  //SelectPoint(-1);

  //watcherWidget->connect(mUI.addPointButton, &QPushButton::pressed, [=]() { AddPoint(); });
  //watcherWidget->connect(mUI.removePointButton, &QPushButton::pressed, [=]() { RemovePoint(); });
  //watcherWidget->connect(mUI.linearCheckBox, &QPushButton::pressed, [=]() { ToggleLinear(); });
}

void DocumentWatcher::OnSlotConnectionChanged(Slot* slot) {
  RefreshGraphList();
}

void DocumentWatcher::RefreshGraphList() {
  Document* doc = static_cast<Document*>(mNode);
  mModel->clear();
  for (Node* node : doc->mGraphs.GetMultiNodes()) {
    Graph* graph = static_cast<Graph*>(node);
    QStandardItem* item = new QStandardItem(QString("graph"));
    item->setData(QVariant::fromValue(graph), GraphNodeRole);
    mModel->appendRow(item);
    //GraphNode* p = index.data(GraphNodeRole).value<GraphNode*>();
  }
}


