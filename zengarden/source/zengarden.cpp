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

static ZenGarden* gZengarden;

ZenGarden::ZenGarden(QWidget *parent)
  : QMainWindow(parent)
  , mNextGraphIndex(0)
  , mPropertyEditor(nullptr)
  , mPropertyLayout(nullptr) {
  gZengarden = this;
  mUI.setupUi(this);

  connect(mUI.upperLeftPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    delete mUI.upperLeftPanel->widget(index);
  });
  connect(mUI.bottomLeftPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    if (index > 0) delete mUI.bottomLeftPanel->widget(index);
  });
  connect(mUI.upperRightPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    delete mUI.upperRightPanel->widget(index);
  });

  connect(mUI.actionSaveAs, SIGNAL(triggered()), this, SLOT(HandleMenuSaveAs()));
  connect(mUI.actionNew, SIGNAL(triggered()), this, SLOT(HandleMenuNew()));
  connect(mUI.actionOpen, SIGNAL(triggered()), this, SLOT(HandleMenuOpen()));
  QTimer::singleShot(0, this, SLOT(InitModules()));
}

ZenGarden::~ZenGarden() {
  DisposeModules();
}

ZenGarden* ZenGarden::GetInstance() {
  return gZengarden;
}

void ZenGarden::InitModules() {
  mLogWatcher = new LogWatcher(this);
  mUI.bottomLeftPanel->addTab(mLogWatcher, "Log");
  mPropertyLayout = new QVBoxLayout(mUI.propertyPanel);

  mTime.start();
  QTimer::singleShot(0, this, SLOT(UpdateTimeNode()));

  /// Set palette
  //QPalette pal = mUI.dummy->palette();
  //QPalette pal2 = mUI.dummy3->palette();
  //pal.setColor(QPalette::Background, pal.background().color().light(125));
  //pal.setColor(QPalette::WindowText, pal.background().color().light(135));
  //mUI.dummy->setPalette(pal);
  //pal.setColor(QPalette::WindowText, pal.background().color().dark());
  //mUI.dummy2->setPalette(pal);
  //pal2.setColor(QPalette::WindowText, QColor(200, 200, 200));
  //mUI.dummy3->setPalette(pal2);
  //mUI.dummy->repaint();

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
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
  Watch(graph, WatcherPosition::RIGHT_TAB);
}

void ZenGarden::DisposeModules() {
  /// Close all watchers
  while (mUI.upperLeftPanel->count() > 0) delete mUI.upperLeftPanel->widget(0);
  while (mUI.bottomLeftPanel->count() > 0) delete mUI.bottomLeftPanel->widget(0);
  while (mUI.upperRightPanel->count() > 0) delete mUI.upperRightPanel->widget(0);
  while (mUI.bottomRightPanel->count() > 0) delete mUI.bottomRightPanel->widget(0);

  Prototypes::Dispose();
  DisposePainter();
  CloseZengine();
}


void ZenGarden::NewGraph() {
  Graph* graph = new Graph();
  mDocument->mGraphs.Connect(graph);
}


void ZenGarden::RestartSceneTimer() {
  mSceneStartTime = mTime.elapsed() - int(TheSceneTime->Get() * 1000.0f);
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

void ZenGarden::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Space:
      RestartSceneTimer();
      mPlayScene = !mPlayScene;
      return;
    case Qt::Key_Escape:
      TheSceneTime->Set(0);
      RestartSceneTimer();
      return;
  }
  QMainWindow::keyPressEvent(event);
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
    shared_ptr<WatcherUI> watcher;
    if (IsInstanceOf<FloatNode>(node)) {
      watcher = node->Watch<StaticFloatEditor>(static_cast<FloatNode*>(node));
    } else if (IsInstanceOf<Vec3Node>(node)) {
      watcher = node->Watch<StaticVec3Editor>(static_cast<Vec3Node*>(node));
    } else if (IsInstanceOf<Vec4Node>(node)) {
      watcher = node->Watch<StaticVec4Editor>(static_cast<Vec4Node*>(node));
    } else {
      watcher = node->Watch<DefaultPropertyEditor>(node);
    }
    if (watcher) {
      watcher->onUnwatch = Delegate(this, &ZenGarden::DeleteWatcherWidget);
      mPropertyEditor =
        new WatcherWidget(mUI.propertyPanel, watcher, WatcherPosition::PROPERTY_PANEL);
      watcher->SetWatcherWidget(mPropertyEditor);
      mPropertyLayout->addWidget(mPropertyEditor);
    }
  }
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
      tabWidget = mUI.upperRightPanel;
      break;
    default: SHOULD_NOT_HAPPEN; break;
  }

  WatcherWidget* watcherWidget = nullptr;
  shared_ptr<WatcherUI> watcher;

  /// Non-3D watchers
  switch (node->GetType()) {
    case NodeType::STRING:
    {
      auto stringNode = dynamic_cast<StringNode*>(node);
      watcher = stringNode->Watch<TextWatcher>(stringNode);
      watcherWidget = new WatcherWidget(tabWidget, watcher, watcherPosition, tabWidget);
      break;
    }
    default: break;
  }

  if (watcherWidget == nullptr) {
    NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(node);
    if (nodeClass->mClassName == "Float Spline") {
      watcher = node->Watch<FloatSplineWatcher>(dynamic_cast<SSpline*>(node));
      watcherWidget = new WatcherWidget(tabWidget, watcher, watcherPosition, tabWidget);
      dynamic_pointer_cast<FloatSplineWatcher>(watcher)->OnAdjustTime +=
        Delegate(this, &ZenGarden::RestartSceneTimer);
    }
  }

  /// 3D watchers
  if (watcherWidget == nullptr) {
    switch (node->GetType()) {
      case NodeType::PASS:
      {
        auto passNode = dynamic_cast<Pass*>(node);
        watcher = static_pointer_cast<WatcherUI>(passNode->Watch<PassWatcher>(passNode));
      }
      break;
      case NodeType::MESH:
      {
        auto meshNode = dynamic_cast<MeshNode*>(node);
        watcher = static_pointer_cast<WatcherUI>(meshNode->Watch<MeshWatcher>(meshNode));
      }
      break;
      case NodeType::DRAWABLE:
      {
        auto drawableNode = dynamic_cast<Drawable*>(node);
        watcher =
          static_pointer_cast<WatcherUI>(drawableNode->Watch<DrawableWatcher>(drawableNode));
      }
      break;
      case NodeType::SCENE:
      {
        auto sceneNode = dynamic_cast<SceneNode*>(node);
        watcher = static_pointer_cast<WatcherUI>(sceneNode->Watch<SceneWatcher>(sceneNode));
      }
      break;
      case NodeType::GRAPH:
      {
        auto graphNode = dynamic_cast<Graph*>(node);
        watcher = static_pointer_cast<WatcherUI>(graphNode->Watch<GraphWatcher>(graphNode));
      }
      break;
      default: return;
    }
    watcherWidget =
      new GLWatcherWidget(tabWidget, watcher, mCommonGLWidget, watcherPosition, tabWidget);
  }

  int index = tabWidget->addTab(watcherWidget, watcher->GetDisplayedName());
  tabWidget->setCurrentIndex(index);
  watcher->SetWatcherWidget(watcherWidget);
  watcher->onUnwatch = Delegate(this, &ZenGarden::DeleteWatcherWidget);
}


void ZenGarden::DeleteWatcherWidget(WatcherWidget* widget) {
  if (widget->mTabWidget) {
    int index = widget->mTabWidget->indexOf(widget);
    ASSERT(index >= 0);
    widget->mTabWidget->removeTab(index);
  }
  if (widget == mPropertyEditor) {
    mPropertyEditor = nullptr;
  }
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
  if (mPlayScene) {
    TheSceneTime->Set(float(mTime.elapsed() - mSceneStartTime) / 1000.0f);
  }
  QTimer::singleShot(10, this, SLOT(UpdateTimeNode()));
}

void ZenGarden::HandleMenuNew() {
  DeleteDocument();
  mDocument = new Document();
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
