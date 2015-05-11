#include "zengarden.h"
#include "util/uipainter.h"
#include "util/util.h"
#include "commands/command.h"
#include "commands/graphCommands.h"
#include "graph/prototypes.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QTime>


ZenGarden::ZenGarden(QWidget *parent)
	: QMainWindow(parent)
	, NextGraphIndex(0)
	, PropEditor(nullptr)
{
	ui.setupUi(this);

	//TheLogger->OnLog += Delegate(this, &ZenGarden::Log);

	connect(ui.addGraphButton, SIGNAL(clicked()), this, SLOT(NewGraph()));
	QTimer::singleShot(0, this, SLOT(InitModules()));
}

ZenGarden::~ZenGarden()
{
	DisposeModules();
}

void ZenGarden::InitModules()
{
	TheLogWatcher = new LogWatcher(this);
	ui.leftPanel->addTab(TheLogWatcher, "Log");

	/// Set palette
	QPalette pal = ui.dummy->palette();
	QPalette pal2 = ui.dummy3->palette();
	pal.setColor(QPalette::Background, pal.background().color().light(125));
	pal.setColor(QPalette::WindowText, pal.background().color().light(135));
	ui.dummy->setPalette(pal);
	pal.setColor(QPalette::WindowText, pal.background().color().dark());
	ui.dummy2->setPalette(pal);
	pal2.setColor(QPalette::WindowText, QColor(200, 200, 200));
	ui.dummy3->setPalette(pal2);
	ui.dummy->repaint();

	/// Initialize OpenGL and its dependencies
	CommonGLWidget = new QGLWidget();
	if (!CommonGLWidget->isValid())
	{
		ERR("No GL context!");
	}
	CommonGLWidget->makeCurrent();
	InitZengine();
	InitPainter();
	Prototypes::Init();

	/// Create blank document
	Doc = new Document();
	DocWatcher = new DocumentWatcher(ui.graphsListView, Doc);

	GraphNode* graph = new GraphNode();

	Doc->Graphs.Connect(graph);
	GraphEditor* graphEditor = OpenGraphViewer(false, graph);

	/// TEST
	char* testShaderStubSource = ReadFileQt("test2.fs");
	auto x = new ShaderStub(testShaderStubSource);

	/// Add some dummy nodes
	ShaderNode* shaderOp = LoadShader("test.vs", "test.fs");
	if (shaderOp)
	{
		TheCommandStack->Execute(new CreateNodeCommand(shaderOp, graphEditor));
		NodeWidget* ow = graphEditor->GetNodeWidget(shaderOp);
		TheCommandStack->Execute(new MoveNodeCommand(ow, Vec2(20, 50)));
	}

	RenderableNode* model = new RenderableNode();
	TheCommandStack->Execute(new CreateNodeCommand(model, graphEditor));
	NodeWidget* ow = graphEditor->GetNodeWidget(model);
	TheCommandStack->Execute(new MoveNodeCommand(ow, Vec2(ADJUST(250), 70)));

	//model->TheShader.Connect(shaderOp);

	/// Build main screen
	//TheGraphEditor->update();
}

void ZenGarden::DisposeModules()
{
	Prototypes::Dispose();
	DisposePainter();
	CloseZengine();
}

GraphEditor* ZenGarden::OpenGraphViewer(bool LeftPanel, GraphNode* Graph)
{
	QTabWidget* tabWidget = LeftPanel ? ui.leftPanel : ui.rightPanel;
	WatcherPosition position = LeftPanel ? WatcherPosition::LEFT_TAB : WatcherPosition::RIGHT_TAB;
	WatcherWidget* watcherWidget = new WatcherWidget(tabWidget, position, true);
	watcherWidget->OnSelectNode += Delegate(this, &ZenGarden::SetNodeForPropertyEditor);

	GraphEditor* graphEditor = new GraphEditor(watcherWidget, CommonGLWidget);
	graphEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	tabWidget->addTab(graphEditor, "graph");

	graphEditor->SetGraph(Graph);
	return graphEditor;
}

void ZenGarden::NewGraph()
{
	GraphNode* graph = new GraphNode();
	graph->Name = string("Graph ") + to_string(++NextGraphIndex);
	Doc->Graphs.Connect(graph);
}

void ZenGarden::SetNodeForPropertyEditor(Node* Nd)
{
	SafeDelete(PropEditor);
	if (Nd != nullptr) {
		PropEditor = PropertyEditor::ForNode(Nd, ui.propertyPanel);
	}
}
