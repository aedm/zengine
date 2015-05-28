#include "watcherwidget.h"
#include <QtWidgets/QBoxLayout>

WatcherWidget::WatcherWidget(QWidget* Parent, WatcherPosition _Position, bool _CanBeClosed)
	: QWidget(Parent)
	, Position(_Position)
	, Watcher(nullptr)
	, CanBeClosed(_CanBeClosed)
{
}

WatcherWidget::~WatcherWidget()
{
	SafeDelete(Watcher);
}



GLWatcherWidget::GLWatcherWidget(QWidget* Parent, QGLWidget* ShareWidget, WatcherPosition Position)
	: WatcherWidget(Parent, Position, true)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(4, 4, 4, 4);

	GLWidget = new QGLWidget(this, ShareWidget);
	layout->addWidget(GLWidget);
}


GLWatcherWidget::~GLWatcherWidget()
{}
