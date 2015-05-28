#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>

enum class WatcherPosition 
{
	LEFT_TAB,
	RIGHT_TAB,
};

class WatcherWidget : public QWidget
{
public:
	WatcherWidget(QWidget* Parent, WatcherPosition Position, bool CanBeClosed);
	virtual ~WatcherWidget();

	Node*							Watcher;

	/// Triggered when a new node needs to be watched
	Event<Node*, WatcherWidget*>	OnWatchNode;
	Event<Node*>					OnSelectNode;

	WatcherPosition					Position;
	bool							CanBeClosed;
};

class GLWatcherWidget : public WatcherWidget
{
public:
	GLWatcherWidget(QWidget* Parent, QGLWidget* ShareWidget, WatcherPosition Position);
	virtual ~GLWatcherWidget();

	QGLWidget*						GLWidget;
};