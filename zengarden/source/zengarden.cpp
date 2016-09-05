#include "zengarden.h"
#include "util/uipainter.h"
#include "util/util.h"
#include "commands/command.h"
#include "commands/graphCommands.h"
#include "graph/prototypes.h"
#include "watchers/passwatcher.h"
#include "watchers/meshwatcher.h"
#include "watchers/scenewatcher.h"
#include "watchers/drawablewatcher.h"
#include "watchers/textwatcher.h"
#include "watchers/splinewatcher.h"
#include "propertyeditor/propertyeditor.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QMouseEvent>
#include <QFileDialog>

ZenGarden::ZenGarden(QWidget *parent)
  : QMainWindow(parent)
  , mNextGraphIndex(0)
  , mPropertyEditor(nullptr)
  , mPropertyLayout(nullptr) {
  mUI.setupUi(this);

  connect(mUI.upperLeftPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    if (index>0) delete mUI.upperLeftPanel->widget(index);
  });
  connect(mUI.bottomLeftPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    delete mUI.bottomLeftPanel->widget(index);
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
  mUI.bottomLeftPanel->addTab(mLogWatcher, "Log");
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

  /// Load Zengine files
  QString engineShadersFolder("engine/common/");
  connect(&mEngineShadersFolderWatcher, SIGNAL(fileChanged(const QString&)),
          this, SLOT(LoadEngineShader(const QString&)));
  LoadEngineShaders(engineShadersFolder);

  InitPainter();
  Prototypes::Init();
  GeneralSceneWatcher::Init();

  /// Create blank document
  mDocument = new Document();
  mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
  Watch(graph, WatcherPosition::RIGHT_TAB);
}

void ZenGarden::DisposeModules() {
  /// Close all watchers
  while (mUI.upperLeftPanel->count() > 0) delete mUI.upperLeftPanel->widget(0);
  while (mUI.bottomLeftPanel->count() > 0) delete mUI.bottomLeftPanel->widget(0);
  while (mUI.rightPanel->count() > 0) delete mUI.rightPanel->widget(0);
  
  Prototypes::Dispose();
  DisposePainter();
  CloseZengine();
}


void ZenGarden::NewGraph() {
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
}


void ZenGarden::LoadEngineShaders(QString& path) {
  static const QString shaderSuffix("shader");

  QDir dir(path);
  for (QFileInfo& fileInfo : dir.entryInfoList()) {
    if (!fileInfo.isDir() && fileInfo.completeSuffix() == shaderSuffix) {
      QString filePath = fileInfo.absoluteFilePath();
      LoadEngineShader(filePath);
      mEngineShadersFolderWatcher.addPath(filePath);
    }
  }
  TheEngineStubs->OnLoadFinished();
}

void ZenGarden::LoadEngineShader(const QString& path) {
  QFileInfo fileInfo(path);
  INFO("loading '%s'", fileInfo.fileName().toLatin1().data());
  unique_ptr<char> stubSource(Util::ReadFileQt(path));
  TheEngineStubs->SetStubSource(fileInfo.baseName().toStdString(),
                                string(stubSource.get()));
}

void ZenGarden::SetNodeForPropertyEditor(Node* node) {
  SafeDelete(mPropertyEditor);
  if (node != nullptr) {
    mPropertyEditor =
      new WatcherWidget(mUI.propertyPanel, WatcherPosition::PROPERTY_PANEL);
    mPropertyEditor->onWatchNode += Delegate(this, &ZenGarden::Watch);
    mPropertyEditor->onWatcherDeath = Delegate(this, &ZenGarden::RemovePropertyEditor);
    mPropertyLayout->addWidget(mPropertyEditor);

    if (IsInstanceOf<FloatNode>(node)) {
      new StaticFloatEditor(static_cast<FloatNode*>(node), mPropertyEditor);
    } 
    else if (IsInstanceOf<Vec3Node>(node)) {
      new StaticVec3Editor(static_cast<Vec3Node*>(node), mPropertyEditor);
    } 
    else if (IsInstanceOf<Vec4Node>(node)) {
      new StaticVec4Editor(static_cast<Vec4Node*>(node), mPropertyEditor);
    } else {
      new DefaultPropertyEditor(node, mPropertyEditor);
    }
  }
}


void ZenGarden::RemovePropertyEditor(WatcherWidget* watcherWidget) {
  ASSERT(watcherWidget == mPropertyEditor);
  SafeDelete(mPropertyEditor);
}


void ZenGarden::Watch(Node* node, WatcherPosition watcherPosition) {
  QTabWidget* tabWidget = nullptr;
  switch (watcherPosition) {
    case WatcherPosition::UPPER_LEFT_TAB:
      tabWidget = mUI.upperLeftPanel;
      break;
    case WatcherPosition::BOTTOM_LEFT_TAB:
      tabWidget = mUI.bottomLeftPanel;
      break;
    case WatcherPosition::RIGHT_TAB:
      tabWidget = mUI.rightPanel;
      break;
    default: SHOULDNT_HAPPEN; break;
  }
  
  WatcherWidget* watcherWidget = nullptr;
  Watcher* watcher = nullptr;

  /// Non-3D watchers
  switch (node->GetType()) {
    case NodeType::STRING:
      watcherWidget = new WatcherWidget(tabWidget, watcherPosition, tabWidget);
      watcher = new TextWatcher(dynamic_cast<StringNode*>(node), watcherWidget);
      break;
    default: break;
  }

  if (watcherWidget == nullptr) {
    NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(node);
    if (nodeClass->mClassName == "Float Spline") {
      watcherWidget = new WatcherWidget(tabWidget, watcherPosition, tabWidget);
      watcher = new FloatSplineWatcher(dynamic_cast<SSpline*>(node), watcherWidget);
    }
  }

  /// 3D watchers
  if (watcherWidget == nullptr) {
    GLWatcherWidget* glWatcherWidget =
      new GLWatcherWidget(tabWidget, mCommonGLWidget, watcherPosition, tabWidget);
    watcherWidget = glWatcherWidget;
    switch (node->GetType()) {
      case NodeType::PASS:
        watcher = new PassWatcher(dynamic_cast<Pass*>(node), glWatcherWidget);
        break;
      case NodeType::MESH:
        watcher = new MeshWatcher(dynamic_cast<MeshNode*>(node), glWatcherWidget);
        break;
      case NodeType::DRAWABLE:
        watcher = new DrawableWatcher(dynamic_cast<Drawable*>(node), glWatcherWidget);
        break;
      case NodeType::SCENE:
        watcher = new SceneWatcher(dynamic_cast<SceneNode*>(node), glWatcherWidget);
        break;
      case NodeType::GRAPH:
        glWatcherWidget->onSelectNode += Delegate(this, &ZenGarden::SetNodeForPropertyEditor);
        glWatcherWidget->onWatchNode += Delegate(this, &ZenGarden::Watch);
        glWatcherWidget->onWatcherDeath = Delegate(this, &ZenGarden::CloseWatcherTab);
        watcher = new GraphWatcher(dynamic_cast<Graph*>(node), glWatcherWidget);
        break;
      default: return;
    }
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


void ZenGarden::HandleMenuSaveAs() {
  QString fileName = QFileDialog::getSaveFileName(this,
      tr("Open project"), "app", tr("Zengine project (*.zen)"));

  INFO("Saving document...");
  QTime myTimer;
  myTimer.start();
  string json = ToJSON(mDocument);
  QFile file(fileName);
  file.open(QIODevice::WriteOnly);
  file.write(json.c_str());

  int milliseconds = myTimer.elapsed();
  INFO("Document saved in %.3f seconds.", float(milliseconds) / 1000.0f);
  mDocumentFileName = fileName;
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
  Watch(graph, WatcherPosition::RIGHT_TAB);
}

void ZenGarden::HandleMenuOpen() {
  QString fileName = QFileDialog::getOpenFileName(this,
      tr("Open project"), "app", tr("Zengine project (*.zen)"));
  if (fileName.isEmpty()) return;

  /// Measure load time
  QTime myTimer;
  myTimer.start();

  /// Load file content
  unique_ptr<char> json = unique_ptr<char>(Util::ReadFileQt(fileName));
  if (json == nullptr) return;

  /// Parse file into a Document
  mCommonGLWidget->makeCurrent();
  Document* document = FromJSON(string(json.get()));
  if (document == nullptr) return;

  /// Load succeeded, remove old document
  DeleteDocument();
  mDocument = document;
  mDocumentFileName = fileName;
  mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);

  /// Open first graph
  Graph* graph = static_cast<Graph*>(mDocument->mGraphs[0]);
  Watch(graph, WatcherPosition::RIGHT_TAB);

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
