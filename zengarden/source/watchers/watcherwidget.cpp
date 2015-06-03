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

	TheGLWidget = new GLWidget(this, ShareWidget);
	layout->addWidget(TheGLWidget);
}


GLWatcherWidget::~GLWatcherWidget()
{}

GLWidget::GLWidget(QWidget* Parent, QGLWidget* ShareWidget)
	: QGLWidget(Parent, ShareWidget)
{}

GLWidget::~GLWidget()
{}

void GLWidget::mouseMoveEvent(QMouseEvent* event)
{
	OnMouseMove(this, event);
}

void GLWidget::mousePressEvent(QMouseEvent* event)
{
	OnMousePress(this, event);
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	OnMouseRelease(this, event);
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
	OnKeyPress(this, event);
}

void GLWidget::keyReleaseEvent(QKeyEvent* event)
{
	OnKeyRelease(this, event);
}

void GLWidget::paintGL()
{
	OnPaint(this);
}

