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

void ZenGarden::Log(LogMessage Message)
{
	//switch (Message.Severity)
	//{
	//case LOG_INFO:		ui.LogViewer->setTextColor(Qt::black);	break;
	//case LOG_WARNING:	ui.LogViewer->setTextColor(Qt::blue);	break;
	//case LOG_ERROR:		ui.LogViewer->setTextColor(Qt::red);	break;
	//}
	//ui.LogViewer->append(QString("[ ") + QTime::currentTime().toString("HH:mm:ss") + QString(" ]  ") + QString::fromUtf16((ushort*)Message.Message));
	//ui.LogViewer->repaint();
}

GraphEditor* ZenGarden::OpenGraphViewer(bool LeftPanel, GraphNode* Graph)
{
	QTabWidget* tabWidget = LeftPanel ? ui.leftPanel : ui.rightPanel;
	GraphEditor* graphEditor = new GraphEditor(tabWidget, CommonGLWidget);
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
