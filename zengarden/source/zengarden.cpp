#include "zengarden.h"
#include "util/glPainter.h"
#include "commands/command.h"
#include "commands/graphCommands.h"
#include <zengine.h>
#include <QtCore/QTimer>
#include <QtCore/QTime>


zengarden::zengarden(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	OpPanel = new OperatorPanel(ui.tab);
	OpPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ui.leftPanelFirstTabLayout->addWidget(OpPanel);

	TheLogger->OnLog += MakeDelegate(this, &zengarden::Log);

	QTimer::singleShot(0, this, SLOT(InitModules()));

}

zengarden::~zengarden()
{
	DisposeModules();
}

void zengarden::InitModules()
{
	InitZengine();
	//InitEditorComponents();
	InitCanvas();

	Doc = new Document();
	OperatorGraph* graph = new OperatorGraph();
	OpPanel->SetGraph(graph);

	ShaderOperator* shaderOp = LoadShader("test.vs", "test.fs");
	if (shaderOp)
	{
		TheCommandStack->Execute(new CreateOperatorCommand(shaderOp, OpPanel));
		OperatorWidget* ow = OpPanel->GetOperatorWidget(shaderOp);
		TheCommandStack->Execute(new MoveOperatorCommand(ow, Vec2(20, 50)));
	}

	Model* model = new Model();
	TheCommandStack->Execute(new CreateOperatorCommand(model, OpPanel));
	OperatorWidget* ow = OpPanel->GetOperatorWidget(model);
	TheCommandStack->Execute(new MoveOperatorCommand(ow, Vec2(ADJUST(250), 70)));

	//model->TheShader.Connect(shaderOp);
	OpPanel->update();
}

void zengarden::DisposeModules()
{
	//CloseEditorComponents();
	CloseZengine();
}

void zengarden::Log(LogMessage Message)
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
