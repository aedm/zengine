#include "zengarden.h"
#include "util/uipainter.h"
#include "util/util.h"
#include "commands/command.h"
#include "graph/prototypes.h"
#include "watchers/passwatcher.h"
#include "watchers/meshwatcher.h"
#include "watchers/scenewatcher.h"
#include "watchers/drawablewatcher.h"
#include "graph/graphwatcher.h"
#include "watchers/textwatcher.h"
#include "watchers/splinewatcher.h"
#include "watchers/moviewatcher.h"
#include "watchers/timelineeditor.h"
#include "propertyeditor/propertyeditor.h"
#include "propertyeditor/sloteditor.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QMouseEvent>
#include <QFileDialog>
#include <bass.h>

static ZenGarden* gZengarden;

ZenGarden::ZenGarden(QWidget *parent)
  : QMainWindow(parent)
{
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

void ZenGarden::OpenGLMakeCurrent()
{
  GetInstance()->mCommonGLWidget->makeCurrent();
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
  EngineStubs::OnLoadFinished();

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
  mCommonGLWidget->makeCurrent();
  DisposePainter();
  CloseZengine();
}

float ZenGarden::GetElapsedBeats() const
{
  const float bps = GetPropertiesNode()->mBPM.Get() / 60.0f;
  const int elapsed = mTime.elapsed();
  const float beats = bps * float(elapsed) / 1000.0f;
  return beats;
}

void ZenGarden::RestartMovieTimer() {
  mMovieStartBeat = GetElapsedBeats() - mMovieCursor;
  mOnMovieCursorChange(mMovieCursor);
  if (mPlayMovie) PlayMusic(mMovieCursor);
}

void ZenGarden::LoadEngineShaders(const QDir& dir) {
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
  case Qt::Key_F8:
    mUI.upperRightPanel->setVisible(!mUI.upperRightPanel->isVisible());
    return;
  case Qt::Key_F7:
    mUI.bottomLeftPanel->setVisible(!mUI.bottomLeftPanel->isVisible());
    return;
  default: break;
  }
  QMainWindow::keyPressEvent(event);
}

void ZenGarden::LoadEngineShader(const QString& path) const
{
  const QFileInfo fileInfo(path);
  INFO("loading '%s'", fileInfo.fileName().toLatin1().data());
  const unique_ptr<char> stubSource(Util::ReadFileQt(path));
  const QString relativePath = mEngineShadersDir.relativeFilePath(path);
  const QString stubName = relativePath.left(relativePath.lastIndexOf("."));
  TheEngineStubs->SetStubSource(stubName.toStdString(),
    string(stubSource.get()));
}

void ZenGarden::SetNodeForPropertyEditor(const shared_ptr<Node>& node) {
  SafeDelete(mPropertyEditor);
  if (node != nullptr) {
    shared_ptr<WatcherUi> watcher;
    if (IsExactType<FloatNode>(node)) {
      watcher =
        node->Watch<StaticValueWatcher<ValueType::FLOAT>>(PointerCast<FloatNode>(node));
    }
    else if (IsExactType<Vec2Node>(node)) {
      watcher =
        node->Watch<StaticValueWatcher<ValueType::VEC2>>(PointerCast<Vec2Node>(node));
    }
    else if (IsExactType<Vec3Node>(node)) {
      watcher =
        node->Watch<StaticValueWatcher<ValueType::VEC3>>(PointerCast<Vec3Node>(node));
    }
    else if (IsExactType<Vec4Node>(node)) {
      watcher =
        node->Watch<StaticValueWatcher<ValueType::VEC4>>(PointerCast<Vec4Node>(node));
    }
    else if (IsPointerOf<Pass>(node)) {
      watcher = node->Watch<PassSlotEditor>(node);
    }
    else {
      watcher = node->Watch<SlotEditor>(node);
    }
    if (watcher) {
      watcher->mDeleteWatcherWidgetCallback = Delegate(this, &ZenGarden::DeleteWatcherWidget);
      mPropertyEditor =
        new WatcherWidget(mUI.propertyPanel, watcher, WatcherPosition::PROPERTY_PANEL);
      watcher->SetWatcherWidget(mPropertyEditor);
      mPropertyLayout->addWidget(mPropertyEditor);
    }
  }
}


shared_ptr<Node> ZenGarden::GetNodeInPropertyEditor() const
{
  if (!mPropertyEditor) return nullptr;
  return mPropertyEditor->mWatcher->GetDirectNode();
}


void ZenGarden::SetMovieCursor(float beats) {
  mMovieCursor = beats;
  RestartMovieTimer();
}

float ZenGarden::GetGlobalTime() const
{
  return GetElapsedBeats();
}

float ZenGarden::GetMovieCursor() const
{
  return mMovieCursor;
}

shared_ptr<PropertiesNode> ZenGarden::GetPropertiesNode() const
{
  return mDocument->mProperties.GetNode();
}

std::shared_ptr<MovieNode> ZenGarden::GetMovieNode() const
{
  return mDocument->mMovie.GetNode();
}

void ZenGarden::Watch(const shared_ptr<Node>& node, WatcherPosition watcherPosition) {
  const shared_ptr<Node> refNode = node->GetReferencedNode();
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
  shared_ptr<WatcherUi> watcher;

  /// Non-3D watchers
  if (IsPointerOf<StringNode>(refNode)) {
    watcher = node->Watch<TextWatcher>(node);
  }
  else if (IsPointerOf<Document>(refNode)) {
    watcher = node->Watch<DocumentWatcher>(node);
  }
  else if (IsPointerOf<FloatSplineNode>(refNode)) {
    watcher = node->Watch<FloatSplineWatcher>(node);
  }

  if (watcher) {
    /// Create non-3D watcher widget
    watcherWidget = new WatcherWidget(tabWidget, watcher, watcherPosition, tabWidget);
  }

  /// 3D watchers
  if (IsPointerOf<Pass>(refNode)) {
    watcher = node->Watch<PassWatcher>(node);
  }
  else if (IsPointerOf<MeshNode>(refNode)) {
    watcher = node->Watch<MeshWatcher>(node);
  }
  else if (IsPointerOf<Drawable>(refNode)) {
    watcher = node->Watch<DrawableWatcher>(node);
  }
  else if (IsPointerOf<SceneNode>(refNode)) {
    watcher = node->Watch<SceneWatcher>(node);
  }
  else if (IsPointerOf<Graph>(refNode)) {
    watcher = node->Watch<GraphWatcher>(node);
  }
  else if (IsPointerOf<MovieNode>(refNode)) {
    watcher = node->Watch<MovieWatcher>(node);
  }

  if (!watcher) {
    INFO("Naaananana... can't watch this.");
    return;
  }

  if (watcherWidget == nullptr) {
    watcherWidget =
      new GLWatcherWidget(tabWidget, watcher, mCommonGLWidget, watcherPosition, tabWidget);
  }

  const int index = tabWidget->addTab(watcherWidget, watcher->GetDisplayedName());
  tabWidget->setCurrentIndex(index);
  watcher->SetWatcherWidget(watcherWidget);
  watcher->mDeleteWatcherWidgetCallback = Delegate(this, &ZenGarden::DeleteWatcherWidget);
}


void ZenGarden::DeleteWatcherWidget(WatcherWidget* widget) {
  if (widget->mTabWidget) {
    const int index = widget->mTabWidget->indexOf(widget);
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
  mDocument = make_shared<Document>();
  mDocument->mProperties.Connect(make_shared<PropertiesNode>());
  mDocument->mMovie.Connect(make_shared<MovieNode>());

  const shared_ptr<Graph> graph = make_shared<Graph>();
  mDocument->mGraphs.Connect(graph);

  Watch(mDocument, WatcherPosition::BOTTOM_LEFT_TAB);
  Watch(graph, WatcherPosition::RIGHT_TAB);

  SetupMovieWatcher();
}

void ZenGarden::SetupMovieWatcher() {
  SafeDelete(mMovieWatcherWidget);
  shared_ptr<MovieNode> movieNode = mDocument->mMovie.GetNode();
  shared_ptr<WatcherUi> watcher = movieNode->Watch<TimelineEditor>(movieNode);
  mMovieWatcherWidget =
    new WatcherWidget(mUI.timelineWidget, watcher, WatcherPosition::TIMELINE_PANEL);
  watcher->SetWatcherWidget(mMovieWatcherWidget);
  mMovieWatcherLayout->addWidget(mMovieWatcherWidget);
}

void ZenGarden::HandleMenuSaveAs() {
  const QString fileName = QFileDialog::getSaveFileName(this,
    tr("Open project"), "app", tr("Zengine project (*.zen)"));

  INFO("Saving document...");
  QTime myTimer;
  myTimer.start();
  const string json = ToJson(mDocument);
  QFile file(fileName);
  file.open(QIODevice::WriteOnly);
  file.write(json.c_str());

  const int milliseconds = myTimer.elapsed();
  INFO("Document saved in %.3f seconds.", float(milliseconds) / 1000.0f);
  mDocumentFileName = fileName;
}

void ZenGarden::Tick() {
  const float elapsedBeats = GetElapsedBeats();
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
  const QString fileName = QFileDialog::getOpenFileName(this,
    tr("Open project"), "app", tr("Zengine project (*.zen)"));
  if (fileName.isEmpty()) return;

  /// Measure load time
  QTime myTimer;
  myTimer.start();

  /// Load file content
  const unique_ptr<char> json = unique_ptr<char>(Util::ReadFileQt(fileName));
  if (json == nullptr) return;

  /// Parse file into a Document
  mCommonGLWidget->makeCurrent();
  const shared_ptr<Document> document = FromJson(string(json.get()));
  if (document == nullptr) return;

  /// Load succeeded, remove old document
  DeleteDocument();
  mDocument = document;
  mDocumentFileName = fileName;

  /// Make sure a MovieNode exists in the document.
  if (!mDocument->mMovie.GetNode()) {
    const shared_ptr<MovieNode> movieNode = make_shared<MovieNode>();
    mDocument->mMovie.Connect(movieNode);
  }

  /// Make sure a PropertiesNode exists in the document.
  if (!mDocument->mProperties.GetNode()) {
    mDocument->mProperties.Connect(make_shared<PropertiesNode>());
  }

  /// Open "debug" node first -- nvidia Nsight workaround, it can only debug the
  /// first OpenGL window
  vector<shared_ptr<Node>> nodes;
  mDocument->GenerateTransitiveClosure(nodes, false);
  for (const auto& node : nodes) {
    if (node->GetName() == "debug") {
      Watch(node, WatcherPosition::UPPER_LEFT_TAB);
      break;
    }
  }

  Watch(mDocument, WatcherPosition::BOTTOM_LEFT_TAB);
  SetupMovieWatcher();

  const int milliseconds = myTimer.elapsed();
  INFO("Document loaded in %.3f seconds.", float(milliseconds) / 1000.0f);
}

void ZenGarden::HandlePropertiesMenu() {
  SetNodeForPropertyEditor(mDocument->mProperties.GetDirectNode());
}

void ZenGarden::DeleteDocument() {
  if (!mDocument) return;
  mCommonGLWidget->makeCurrent();

  vector<shared_ptr<Node>> nodes;
  mDocument->GenerateTransitiveClosure(nodes, false);
  for (UINT i = nodes.size(); i > 0; i--) {
    if (nodes[i - 1].use_count()) nodes[i - 1]->Dispose();
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

  if (!BASS_Init(1, 44100, 0, nullptr, nullptr)) {
    ERR("Can't initialize BASS");
    return;
  }
  mBassMusicChannel = BASS_StreamCreateFile(FALSE, L"demo.mp3", 0, 0, BASS_STREAM_PRESCAN);
}

void ZenGarden::PlayMusic(float beats) const
{
  const float bps = GetPropertiesNode()->mBPM.Get() / 60.0f;
  const float seconds = beats / bps;
  if (mBassMusicChannel < 0) return;
  BASS_ChannelSetPosition(mBassMusicChannel,
    BASS_ChannelSeconds2Bytes(mBassMusicChannel, seconds),
    BASS_POS_BYTE);
  BASS_ChannelPlay(mBassMusicChannel, FALSE);
}

void ZenGarden::StopMusic() const
{
  if (mBassMusicChannel < 0) return;
  BASS_ChannelStop(mBassMusicChannel);
}
