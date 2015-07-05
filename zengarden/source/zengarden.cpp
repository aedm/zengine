#include "zengarden.h"
#include "util/uipainter.h"
#include "util/util.h"
#include "commands/command.h"
#include "commands/graphCommands.h"
#include "graph/prototypes.h"
#include "watchers/passwatcher.h"
#include "propertyeditor/propertyeditor.h"
#include "nodes/stubloader.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QTime>


ZenGarden::ZenGarden(QWidget *parent)
  : QMainWindow(parent)
  , mNextGraphIndex(0)
  , mPropertyWatcher(nullptr)
  , mPropertyLayout(nullptr)
{
	mUI.setupUi(this);

	//TheLogger->OnLog += Delegate(this, &ZenGarden::Log);

	connect(mUI.addGraphButton, SIGNAL(clicked()), this, SLOT(NewGraph()));
	QTimer::singleShot(0, this, SLOT(InitModules()));
}

ZenGarden::~ZenGarden()
{
	DisposeModules();
}

void ZenGarden::InitModules()
{
	mLogWatcher = new LogWatcher(this);
	mUI.leftPanel->addTab(mLogWatcher, "Log");
  mPropertyLayout = new QVBoxLayout(mUI.propertyPanel);

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
	if (!mCommonGLWidget->isValid())
	{
		ERR("No GL context!");
	}
	mCommonGLWidget->makeCurrent();
	InitZengine();
	InitPainter();
	Prototypes::Init();

  /// Initialize engine parts
  StubLoader::LoadStubs();

	/// Create blank document
	mDocument = new Document();
	mDocumentWatcher = new DocumentWatcher(mUI.graphsListView, mDocument);

	Graph* graph = new Graph();

	mDocument->mGraphs.Connect(graph);
	GraphWatcher* graphEditor = OpenGraphViewer(false, graph);

	/// TEST
	{
		/// fragment shader
		char* testShaderStubSource = Util::ReadFileQt("test2.fs");
		auto fragmentStub = new ShaderStub(testShaderStubSource);
		ThePrototypes->AddPrototype(fragmentStub, NodeClass::SHADER_STUB);
		TheCommandStack->Execute(new CreateNodeCommand(fragmentStub, graphEditor));
		TheCommandStack->Execute(new MoveNodeCommand(fragmentStub, Vec2(150, 150)));
		delete testShaderStubSource;

		/// vertex shader
    testShaderStubSource = Util::ReadFileQt("test2.vs");
		auto vertexStub = new ShaderStub(testShaderStubSource);
		ThePrototypes->AddPrototype(vertexStub, NodeClass::SHADER_STUB);
		TheCommandStack->Execute(new CreateNodeCommand(vertexStub, graphEditor));
		TheCommandStack->Execute(new MoveNodeCommand(vertexStub, Vec2(150, 250)));

		/// pass
		auto pass = new Pass();
		pass->mFragmentStub.Connect(fragmentStub);
		pass->mVertexStub.Connect(vertexStub);
		TheCommandStack->Execute(new CreateNodeCommand(pass, graphEditor));
    TheCommandStack->Execute(new MoveNodeCommand(pass, Vec2(280, 200)));

		/// float
		auto floatNode = new FloatNode();
		//fragmentStub->mSlots[0]->Connect(floatNode); /// hack
		TheCommandStack->Execute(new CreateNodeCommand(floatNode, graphEditor));
    TheCommandStack->Execute(new MoveNodeCommand(floatNode, Vec2(20, 150)));

    /// texture
    TextureNode* textureNode = new TextureNode();
    Texture* sampleTexture = CreateSampleTexture();
    textureNode->Set(sampleTexture);
    TheCommandStack->Execute(new CreateNodeCommand(textureNode, graphEditor));
    TheCommandStack->Execute(new MoveNodeCommand(textureNode, Vec2(20, 250)));
  }
}

void ZenGarden::DisposeModules()
{
	Prototypes::Dispose();
	DisposePainter();
	CloseZengine();
}

GraphWatcher* ZenGarden::OpenGraphViewer(bool LeftPanel, Graph* Graph)
{
	QTabWidget* tabWidget = LeftPanel ? mUI.leftPanel : mUI.rightPanel;
	WatcherPosition position = LeftPanel 
		? WatcherPosition::LEFT_TAB : WatcherPosition::RIGHT_TAB;

  GLWatcherWidget* glWatcherWidget =
    new GLWatcherWidget(tabWidget, mCommonGLWidget, position, tabWidget);
	//WatcherWidget* watcherWidget = new WatcherWidget(tabWidget, position);
	glWatcherWidget->onSelectNode += Delegate(this, &ZenGarden::SetNodeForPropertyEditor);
  glWatcherWidget->onWatchNode += Delegate(this, &ZenGarden::Watch);

  GraphWatcher* graphEditor = new GraphWatcher(Graph, glWatcherWidget);
	//graphEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tabWidget->addTab(glWatcherWidget, "graph");

	return graphEditor;
}

void ZenGarden::NewGraph()
{
	Graph* graph = new Graph();
	mDocument->mGraphs.Connect(graph);
}

void ZenGarden::SetNodeForPropertyEditor(Node* Nd)
{
  SafeDelete(mPropertyWatcher);
	if (Nd != nullptr) {
    mPropertyWatcher = 
      new WatcherWidget(mUI.propertyPanel, WatcherPosition::PROPERTY_PANEL);
    mPropertyLayout->addWidget(mPropertyWatcher);

    switch (ThePrototypes->GetNodeClass(Nd)) {
      case NodeClass::STATIC_FLOAT:
        new StaticFloatEditor(static_cast<FloatNode*>(Nd), mPropertyWatcher);
        break;
      default:
        new DefaultPropertyEditor(Nd, mPropertyWatcher);
        break;
    }
	}
}

void ZenGarden::Watch(Node* Nd, WatcherWidget* Widget)
{
	QTabWidget* tabWidget = Widget->mPosition == WatcherPosition::RIGHT_TAB 
		? mUI.leftPanel : mUI.rightPanel;
	WatcherPosition position = Widget->mPosition == WatcherPosition::RIGHT_TAB 
		? WatcherPosition::LEFT_TAB : WatcherPosition::RIGHT_TAB;

	WatcherWidget* watcherWidget = nullptr;
  Watcher* watcher = nullptr;

	switch (Nd->GetType()) {
	case NodeType::PASS:
	{
		GLWatcherWidget* glWatcherWidget =
			new GLWatcherWidget(tabWidget, mCommonGLWidget, position, tabWidget);
		watcher = new PassWatcher(static_cast<Pass*>(Nd), glWatcherWidget);
		watcherWidget = glWatcherWidget;
		break;
	}
	default: return;
	}

	int index = tabWidget->addTab(watcherWidget, watcher->GetDisplayedName());
	tabWidget->setCurrentIndex(index);
}

Texture* ZenGarden::CreateSampleTexture() {
  UINT tmp[256 * 256];
  for (UINT i = 0; i < 256; i++)
    for (UINT o = 0; o < 256; o++) {
      UINT c = i^o;
      tmp[i * 256 + o] = c | (c << 8) | (c << 16);
    }
  return TheResourceManager->CreateTexture(256, 256, TEXELTYPE_RGBA_UINT8, tmp);
}
