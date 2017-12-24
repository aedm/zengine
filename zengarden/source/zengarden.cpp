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
#include "watchers/moviewatcher.h"
#include "watchers/timelineeditor.h"
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
    if (index > 1) delete mUI.bottomLeftPanel->widget(index);
  });
  connect(mUI.upperRightPanel, &QTabWidget::tabCloseRequested, [=](int index) {
    delete mUI.upperRightPanel->widget(index);
  });

  connect(mUI.actionSaveAs, SIGNAL(triggered()), this, SLOT(HandleMenuSaveAs()));
  connect(mUI.actionNew, SIGNAL(triggered()), this, SLOT(HandleMenuNew()));
  connect(mUI.actionOpen, SIGNAL(triggered()), this, SLOT(HandleMenuOpen()));
  connect(mUI.actionDocumentProperties, SIGNAL(triggered()), this, 
          SLOT(HandlePropertiesMenu()));

  mUI.timelineWidget->hide();

  QTimer::singleShot(0, this, SLOT(InitModules()));
}

ZenGarden::~ZenGarden() {
  DeleteDocument();
  DisposeModules();
}

ZenGarden* ZenGarden::GetInstance() {
  return gZengarden;
}

void ZenGarden::InitModules() {
  mLogWatcher = new LogWatcher(this);
  mUI.bottomLeftPanel->addTab(mLogWatcher, "Log");
  mPropertyLayout = new QVBoxLayout(mUI.propertyPanel);
  mMovieWatcherLayout = new QVBoxLayout(mUI.timelineWidget);
  mMovieWatcherLayout->setMargin(0);

  mTime.start();
  QTimer::singleShot(0, this, SLOT(Tick()));

  /// Set palette
  QPalette pal = mUI.innerPropertyFrame->palette();
  //QPalette pal2 = mUI.dummy3->palette();
  pal.setColor(QPalette::Background, pal.background().color().light(125));
  pal.setColor(QPalette::WindowText, pal.background().color().light(135));
  mUI.innerPropertyFrame->setPalette(pal);
  pal.setColor(QPalette::WindowText, pal.background().color().dark());
  mUI.outerPropertyFrame->setPalette(pal);
  //pal2.setColor(QPalette::WindowText, QColor(200, 200, 200));
  //mUI.dummy3->setPalette(pal2);
  mUI.verticalDummy->repaint();

  /// Initialize OpenGL and its dependencies
  mCommonGLWidget = new QGLWidget();
  if (!mCommonGLWidget->isValid()) {
    ERR("No GL context!");
  }
  mCommonGLWidget->makeCurrent();
  InitZengine();

  /// Load Zengine files
  mEngineShadersDir = QDir("engine/main/");
  connect(&mEngineShadersFolderWatcher, SIGNAL(fileChanged(const QString&)),
    this, SLOT(LoadEngineShader(const QString&)));
  LoadEngineShaders(mEngineShadersDir);
  TheEngineStubs->OnLoadFinished();

  InitPainter();
  Prototypes::Init();
  GeneralSceneWatcher::Init();

  CreateNewDocument();
  LoadMusic();
}

void ZenGarden::DisposeModules() {
  BASS_Free();

  /// Close all watchers
  while (mUI.upperLeftPanel->count() > 0) delete mUI.upperLeftPanel->widget(0);
  while (mUI.bottomLeftPanel->count() > 0) delete mUI.bottomLeftPanel->widget(0);
  while (mUI.upperRightPanel->count() > 0) delete mUI.upperRightPanel->widget(0);
  SafeDelete(mMovieWatcherWidget);
  //while (mUI.bottomRightPanel->count() > 0) delete mUI.bottomRightPanel->widget(0);

  Prototypes::Dispose();
  DisposePainter();
  CloseZengine();
}

float ZenGarden::GetElapsedBeats() {
  float bps = GetPropertiesNode()->mBPM.Get() / 60.0f;
  int elapsed = mTime.elapsed();
  float beats = bps * float(elapsed) / 1000.0f;
  return beats;
}

void ZenGarden::RestartMovieTimer() {
  mMovieStartBeat = GetElapsedBeats() - mMovieCursor;
  mOnMovieCursorChange(mMovieCursor);
  if (mPlayMovie) PlayMusic(mMovieCursor);
}

void ZenGarden::LoadEngineShaders(QDir& dir) {
  static const QString shaderSuffix("shader");
  std::string a = dir.absolutePath().toStdString();

  for (QFileInfo& fileInfo : dir.entryInfoList()) {
    if (fileInfo.fileName().startsWith(".")) continue;
    std::string b = fileInfo.absoluteFilePath().toStdString();
    if (fileInfo.isDir()) {
      LoadEngineShaders(QDir(fileInfo.absoluteFilePath()));
    } 
    else if (fileInfo.completeSuffix() == shaderSuffix) {
      QString filePath = fileInfo.absoluteFilePath();
      LoadEngineShader(filePath);
      mEngineShadersFolderWatcher.addPath(filePath);
    }
  }
}


void ZenGarden::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Space:
      mPlayMovie = !mPlayMovie;
      RestartMovieTimer();
      if (!mPlayMovie) StopMusic();
      return;
    case Qt::Key_Escape:
      mMovieCursor = 0.0f;
      RestartMovieTimer();
      return;
    case Qt::Key_5:
      mUI.timelineWidget->setVisible(!mUI.timelineWidget->isVisible());
      return;
    case Qt::Key_F12:
      mUI.upperRightPanel->setVisible(!mUI.upperRightPanel->isVisible());
      return;
    case Qt::Key_F11:
      mUI.bottomLeftPanel->setVisible(!mUI.bottomLeftPanel->isVisible());
      return;
  }
  QMainWindow::keyPressEvent(event);
}

void ZenGarden::LoadEngineShader(const QString& path) {
  QFileInfo fileInfo(path);
  INFO("loading '%s'", fileInfo.fileName().toLatin1().data());
  unique_ptr<char> stubSource(Util::ReadFileQt(path));
  QString relativePath = mEngineShadersDir.relativeFilePath(path);
  QString stubName = relativePath.left(relativePath.lastIndexOf("."));
  TheEngineStubs->SetStubSource(stubName.toStdString(),
    string(stubSource.get()));
}

void ZenGarden::SetNodeForPropertyEditor(Node* node) {
  SafeDelete(mPropertyEditor);
  if (node != nullptr) {
    shared_ptr<WatcherUI> watcher;
    if (IsExactType<FloatNode>(node)) {
      watcher = node->Watch<StaticFloatEditor>(static_cast<FloatNode*>(node));
    } else if (IsExactType<Vec3Node>(node)) {
      watcher = node->Watch<StaticVec3Editor>(static_cast<Vec3Node*>(node));
    } else if (IsExactType<Vec4Node>(node)) {
      watcher = node->Watch<StaticVec4Editor>(static_cast<Vec4Node*>(node));
    } else {
      watcher = node->Watch<DefaultPropertyEditor>(node);
    }
    if (watcher) {
      watcher->deleteWatcherWidgetCallback = Delegate(this, &ZenGarden::DeleteWatcherWidget);
      mPropertyEditor =
        new WatcherWidget(mUI.propertyPanel, watcher, WatcherPosition::PROPERTY_PANEL);
      watcher->SetWatcherWidget(mPropertyEditor);
      mPropertyLayout->addWidget(mPropertyEditor);
    }
  }
}


Node* ZenGarden::GetNodeInPropertyEditor() {
  if (!mPropertyEditor) return nullptr;
  return mPropertyEditor->mWatcher->GetNode();
}


void ZenGarden::SetMovieCursor(float beats) {
  mMovieCursor = beats;
  RestartMovieTimer();
}


void ZenGarden::SetClipCursor(float seconds) {
  // TODO: make this work.

}


float ZenGarden::GetGlobalTime() {
  return GetElapsedBeats();
}

float ZenGarden::GetMovieCursor() {
  return mMovieCursor;
}

void ZenGarden::SetSceneNodeForClip(SceneNode* sceneNode) {
  if (!mMovieWatcherWidget || !mMovieWatcherWidget->mWatcher) return;
  shared_ptr<TimelineEditor> editor = 
    dynamic_pointer_cast<TimelineEditor>(mMovieWatcherWidget->mWatcher);
  if (!editor) return;
  editor->SetSceneNodeForSelectedClip(sceneNode);
}


PropertiesNode* ZenGarden::GetPropertiesNode() {
  return SafeCast<PropertiesNode*>(mDocument->mProperties.GetDirectNode());
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
  if (IsInsanceOf<StringNode*>(node)) {
    auto stringNode = SafeCast<StringNode*>(node);
    watcher = stringNode->Watch<TextWatcher>(stringNode);
  }
  else if (IsInsanceOf<Document*>(node)) {
    auto documentNode = SafeCast<Document*>(node);
    watcher = documentNode->Watch<DocumentWatcher>(documentNode);
  }

  if (watcher) {
    watcherWidget = new WatcherWidget(tabWidget, watcher, watcherPosition, tabWidget);
  } else {
    NodeClass* nodeClass = NodeRegistry::GetInstance()->GetNodeClass(node);
    if (nodeClass->mClassName == "Float Spline") {
      watcher = node->Watch<FloatSplineWatcher>(dynamic_cast<FloatSplineNode*>(node));
      watcherWidget = new WatcherWidget(tabWidget, watcher, watcherPosition, tabWidget);
    }
  }

  /// 3D watchers
  if (IsInsanceOf<Pass*>(node)) {
    auto passNode = SafeCast<Pass*>(node);
    watcher = static_pointer_cast<WatcherUI>(passNode->Watch<PassWatcher>(passNode));
  }
  else if (IsInsanceOf<MeshNode*>(node)) {
    auto meshNode = SafeCast<MeshNode*>(node);
    watcher = static_pointer_cast<WatcherUI>(meshNode->Watch<MeshWatcher>(meshNode));
  }
  else if (IsInsanceOf<Drawable*>(node)) {
    auto drawableNode = SafeCast<Drawable*>(node);
    watcher =
      static_pointer_cast<WatcherUI>(drawableNode->Watch<DrawableWatcher>(drawableNode));
  }
  else if (IsInsanceOf<SceneNode*>(node)) {
    auto sceneNode = dynamic_cast<SceneNode*>(node);
    watcher = static_pointer_cast<WatcherUI>(sceneNode->Watch<SceneWatcher>(sceneNode));
  }
  else if (IsInsanceOf<Graph*>(node)) {
    auto graphNode = dynamic_cast<Graph*>(node);
    watcher = static_pointer_cast<WatcherUI>(graphNode->Watch<GraphWatcher>(graphNode));
  }
  else if (IsInsanceOf<MovieNode*>(node)) {
    auto movieNode = dynamic_cast<MovieNode*>(node);
    watcher = static_pointer_cast<WatcherUI>(movieNode->Watch<MovieWatcher>(movieNode));
  }

  if (watcherWidget == nullptr) {
    watcherWidget =
      new GLWatcherWidget(tabWidget, watcher, mCommonGLWidget, watcherPosition, tabWidget);
  }

  int index = tabWidget->addTab(watcherWidget, watcher->GetDisplayedName());
  tabWidget->setCurrentIndex(index);
  watcher->SetWatcherWidget(watcherWidget);
  watcher->deleteWatcherWidgetCallback = Delegate(this, &ZenGarden::DeleteWatcherWidget);
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


void ZenGarden::CreateNewDocument() {
  DeleteDocument();
  mDocument = new Document();
  mDocument->mProperties.Connect(new PropertiesNode());
  mDocument->mMovie.Connect(new MovieNode());

  Watch(mDocument, WatcherPosition::BOTTOM_LEFT_TAB);
  SetupMovieWatcher();
}

void ZenGarden::SetupMovieWatcher() {
  SafeDelete(mMovieWatcherWidget);
  MovieNode* movieNode = mDocument->mMovie.GetNode();
  shared_ptr<WatcherUI> watcher = movieNode->Watch<TimelineEditor>(movieNode);
  mMovieWatcherWidget =
    new WatcherWidget(mUI.timelineWidget, watcher, WatcherPosition::TIMELINE_PANEL);
  watcher->SetWatcherWidget(mMovieWatcherWidget);
  mMovieWatcherLayout->addWidget(mMovieWatcherWidget);
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

void ZenGarden::Tick() {
  float elapsedBeats = GetElapsedBeats();
  if (mPlayMovie) {
    mMovieCursor = elapsedBeats - mMovieStartBeat;
    mOnMovieCursorChange(mMovieCursor);
  }
  GlobalTimeNode::OnTimeChanged(elapsedBeats);
  QTimer::singleShot(10, this, SLOT(Tick()));
}

void ZenGarden::HandleMenuNew() {
  CreateNewDocument();
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

  /// Make sure a MovieNode exists in the document.
  if (!mDocument->mMovie.GetNode()) {
    MovieNode* movieNode = new MovieNode();
    mDocument->mMovie.Connect(movieNode);
  }

  /// Make sure a PropertiesNode exists in the document.
  if (!mDocument->mProperties.GetNode()) {
    mDocument->mProperties.Connect(new PropertiesNode());
  }

  /// Open "debug" node first -- nvidia Nsight workaround, it can only debug the
  /// first OpenGL window
  vector<Node*> nodes;
  mDocument->GenerateTransitiveClosure(nodes, false);
  for (Node* node : nodes) {
    if (node->GetName() == "debug") {
      Watch(node, WatcherPosition::UPPER_LEFT_TAB);
      break;
    }
  }

  Watch(mDocument, WatcherPosition::BOTTOM_LEFT_TAB);
  SetupMovieWatcher();

  int milliseconds = myTimer.elapsed();
  INFO("Document loaded in %.3f seconds.", float(milliseconds) / 1000.0f);
}

void ZenGarden::HandlePropertiesMenu() {
  SetNodeForPropertyEditor(mDocument->mProperties.GetDirectNode());
}

void ZenGarden::DeleteDocument() {
  if (!mDocument) return;
  mCommonGLWidget->makeCurrent();

  vector<Node*> nodes;
  mDocument->GenerateTransitiveClosure(nodes, false);
  for (UINT i = nodes.size(); i > 0; i--) {
    delete nodes[i - 1];
  }
  mDocument = nullptr;
  mDocumentWatcher = nullptr;
}

void ZenGarden::LoadMusic() {
  BASS_DEVICEINFO di;
  for (int a = 1; BASS_GetDeviceInfo(a, &di); a++) {
    if (di.flags&BASS_DEVICE_ENABLED) // enabled output device
      INFO("dev %d: %s\n", a, di.name);
  }

  if (!BASS_Init(1, 44100, 0, 0, NULL)) {
    ERR("Can't initialize BASS");
    return;
  }
  mBassMusicChannel = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_PRESCAN);
}

void ZenGarden::PlayMusic(float beats) {
  float bps = GetPropertiesNode()->mBPM.Get() / 60.0f;
  float seconds = beats / bps;
  if (mBassMusicChannel < 0) return;
  BASS_ChannelSetPosition(mBassMusicChannel, 
                          BASS_ChannelSeconds2Bytes(mBassMusicChannel, seconds), 
                          BASS_POS_BYTE);
  BASS_ChannelPlay(mBassMusicChannel, FALSE);
}

void ZenGarden::StopMusic() {
  if (mBassMusicChannel < 0) return;
  BASS_ChannelStop(mBassMusicChannel);
}
