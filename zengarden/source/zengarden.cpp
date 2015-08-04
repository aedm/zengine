#include "zengarden.h"
#include "util/uipainter.h"
#include "util/util.h"
#include "commands/command.h"
#include "commands/graphCommands.h"
#include "graph/prototypes.h"
#include "watchers/passwatcher.h"
#include "propertyeditor/propertyeditor.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QMouseEvent>


ZenGarden::ZenGarden(QWidget *parent)
  : QMainWindow(parent)
  , mNextGraphIndex(0)
  , mPropertyEditor(nullptr)
  , mPropertyLayout(nullptr) {
  mUI.setupUi(this);

  connect(mUI.leftPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    if (index>0) delete mUI.leftPanel->widget(index);
  });
  connect(mUI.rightPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    delete mUI.rightPanel->widget(index);
  });

  connect(mUI.addGraphButton, SIGNAL(clicked()), this, SLOT(NewGraph()));
  connect(mUI.actionSaveAs, SIGNAL(triggered()), this, SLOT(HandleMenuSaveAs()));
  connect(mUI.actionNew, SIGNAL(triggered()), this, SLOT(HandleMenuNew()));
  connect(mUI.actionOpen, SIGNAL(triggered()), this, SLOT(HandleMenuOpen()));
  QTimer::singleShot(0, this, SLOT(InitModules()));
}

ZenGarden::~ZenGarden() {
  DisposeModules();
}

void ZenGarden::InitModules() {
  mLogWatcher = new LogWatcher(this);
  mUI.leftPanel->addTab(mLogWatcher, "Log");
  mPropertyLayout = new QVBoxLayout(mUI.propertyPanel);

  mTime.start();
  QTimer::singleShot(0, this, SLOT(UpdateTimeNode()));

  /// Set palette
  QPalette pal = mUI.dummy->palette();
  QPalette pal2 = mUI.dummy3->palette();
  pal.setColor(QPalette::Background, pal.background().color().light(125));
  pal.setColor(QPalette::WindowText, pal.background().color().light(135));
  mUI.dummy->setPalette(pal);
  pal.setColor(QPalette::WindowText, pal.background().color().dark());
  mUI.dummy2->setPalette(pal);
  pal2.setColor(QPalette::WindowText, QColor(200, 200, 200));
  mUI.dummy3->setPalette(pal2);
  mUI.dummy->repaint();

  /// Initialize OpenGL and its dependencies
  mCommonGLWidget = new QGLWidget();
  if (!mCommonGLWidget->isValid()) {
    ERR("No GL context!");
  }
  mCommonGLWidget->makeCurrent();
  InitZengine();
  InitPainter();
  Prototypes::Init();

  /// Create blank document
  mDocument = new Document();
  mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);

  Graph* graph = new Graph();

  mDocument->mGraphs.Connect(graph);
  GraphWatcher* graphEditor = OpenGraphViewer(false, graph);

  /// TEST
  {
    /// texture
    TextureNode* textureNode = new TextureNode();
    Texture* sampleTexture = CreateSampleTexture();
    textureNode->Set(sampleTexture);
    TheCommandStack->Execute(new CreateNodeCommand(textureNode, graph));
    TheCommandStack->Execute(new MoveNodeCommand(textureNode, Vec2(20, 250)));
  }
}

void ZenGarden::DisposeModules() {
  /// Close all watchers
  while (mUI.leftPanel->count() > 0) {
    delete mUI.leftPanel->widget(0);
  }
  while (mUI.rightPanel->count() > 0) {
    delete mUI.rightPanel->widget(0);
  }
  
  Prototypes::Dispose();
  DisposePainter();
  CloseZengine();
}

GraphWatcher* ZenGarden::OpenGraphViewer(bool LeftPanel, Graph* Graph) {
  QTabWidget* tabWidget = LeftPanel ? mUI.leftPanel : mUI.rightPanel;
  WatcherPosition position = LeftPanel
    ? WatcherPosition::LEFT_TAB : WatcherPosition::RIGHT_TAB;

  GLWatcherWidget* glWatcherWidget =
    new GLWatcherWidget(tabWidget, mCommonGLWidget, position, tabWidget);
  //WatcherWidget* watcherWidget = new WatcherWidget(tabWidget, position);
  glWatcherWidget->onSelectNode += Delegate(this, &ZenGarden::SetNodeForPropertyEditor);
  glWatcherWidget->onWatchNode += Delegate(this, &ZenGarden::Watch);
  glWatcherWidget->onWatcherDeath = Delegate(this, &ZenGarden::CloseWatcherTab);

  GraphWatcher* graphEditor = new GraphWatcher(Graph, glWatcherWidget);
  //graphEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  tabWidget->addTab(glWatcherWidget, "graph");

  return graphEditor;
}

void ZenGarden::NewGraph() {
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
}


void ZenGarden::SetNodeForPropertyEditor(Node* node) {
  SafeDelete(mPropertyEditor);
  if (node != nullptr) {
    mPropertyEditor =
      new WatcherWidget(mUI.propertyPanel, WatcherPosition::PROPERTY_PANEL);
    mPropertyEditor->onWatcherDeath = Delegate(this, &ZenGarden::RemovePropertyEditor);
    mPropertyLayout->addWidget(mPropertyEditor);

    if (IsInstanceOf<FloatNode>(node)) {
      new StaticFloatEditor(static_cast<FloatNode*>(node), mPropertyEditor);
    } else {
      new DefaultPropertyEditor(node, mPropertyEditor);
    }
  }
}


void ZenGarden::RemovePropertyEditor(WatcherWidget* watcherWidget) {
  ASSERT(watcherWidget == mPropertyEditor);
  SafeDelete(mPropertyEditor);
}


void ZenGarden::Watch(Node* node, WatcherWidget* sourceWidget) {
  QTabWidget* tabWidget = sourceWidget->mPosition == WatcherPosition::RIGHT_TAB
    ? mUI.leftPanel : mUI.rightPanel;
  WatcherPosition position = sourceWidget->mPosition == WatcherPosition::RIGHT_TAB
    ? WatcherPosition::LEFT_TAB : WatcherPosition::RIGHT_TAB;

  WatcherWidget* watcherWidget = nullptr;
  Watcher* watcher = nullptr;

  switch (node->GetType()) {
    case NodeType::PASS:
    {
      GLWatcherWidget* glWatcherWidget =
        new GLWatcherWidget(tabWidget, mCommonGLWidget, position, tabWidget);
      watcher = new PassWatcher(static_cast<Pass*>(node), glWatcherWidget);
      watcherWidget = glWatcherWidget;
      break;
    }
    default: return;
  }

  int index = tabWidget->addTab(watcherWidget, watcher->GetDisplayedName());
  tabWidget->setCurrentIndex(index);
  watcherWidget->onWatcherDeath = Delegate(this, &ZenGarden::CloseWatcherTab);
}


void ZenGarden::CloseWatcherTab(WatcherWidget* widget) {
  ASSERT(widget->mTabWidget);
  int index = widget->mTabWidget->indexOf(widget);
  ASSERT(index >= 0);
  widget->mTabWidget->removeTab(index);
  delete widget;
}



Texture* ZenGarden::CreateSampleTexture() {
  UINT* tmp = new UINT[256 * 256];
  for (UINT i = 0; i < 256; i++)
    for (UINT o = 0; o < 256; o++) {
      UINT c = i^o;
      tmp[i * 256 + o] = c | (c << 8) | (c << 16);
    }
  return TheResourceManager->CreateTexture(256, 256, TEXELTYPE_RGBA_UINT8, tmp);
}

void ZenGarden::HandleMenuSaveAs() {
  INFO("Saving document...");
  QTime myTimer;
  myTimer.start();
  string json = ToJSON(mDocument);
  QFile file("sample.zen");
  file.open(QIODevice::WriteOnly);
  file.write(json.c_str());

  int milliseconds = myTimer.elapsed();
  INFO("Document saved in %.3f seconds.", float(milliseconds) / 1000.0f);
}

void ZenGarden::UpdateTimeNode() {
  TimeNode::OnTimeChanged(float(mTime.elapsed()) / 1000.0f);
  QTimer::singleShot(10, this, SLOT(UpdateTimeNode()));
}

void ZenGarden::HandleMenuNew() {
  DeleteDocument();
  mDocument = new Document();
  mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
  OpenGraphViewer(false, graph);
}

void ZenGarden::HandleMenuOpen() {
  QTime myTimer;
  myTimer.start();

  DeleteDocument();
  char* json = Util::ReadFileQt("sample.zen");

  mCommonGLWidget->makeCurrent();
  mDocument = FromJSON(string(json));
  mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);
  Graph* graph = static_cast<Graph*>(mDocument->mGraphs[0]);
  OpenGraphViewer(false, graph);
  delete json;

  int milliseconds = myTimer.elapsed();
  INFO("Document loaded in %.3f seconds.", float(milliseconds) / 1000.0f);
}

void ZenGarden::DeleteDocument() {
  vector<Node*> nodes;
  Util::CreateTopologicalOrder(mDocument, nodes);
  for (UINT i = nodes.size(); i > 0; i--) {
    delete nodes[i - 1];
  }
  mDocument = nullptr;
  mDocumentWatcher = nullptr;
}
