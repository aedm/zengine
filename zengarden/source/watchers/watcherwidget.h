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


/// Boilerplate class, converts Qt virtual functions to events :(
class GLWidget: public QGLWidget
{
public:
	GLWidget(QWidget* Parent, QGLWidget* ShareWidget);
	virtual ~GLWidget();

	Event<GLWidget*, QMouseEvent*>	OnMouseMove;
	Event<GLWidget*, QMouseEvent*>	OnMousePress;
	Event<GLWidget*, QMouseEvent*>	OnMouseRelease;
	Event<GLWidget*, QKeyEvent*>	OnKeyPress;
	Event<GLWidget*, QKeyEvent*>	OnKeyRelease;
	Event<GLWidget*>				OnPaint;

protected:
	virtual void					mouseMoveEvent(QMouseEvent* event) override;
	virtual void					mousePressEvent(QMouseEvent* event) override;
	virtual void					mouseReleaseEvent(QMouseEvent* event) override;
	virtual void					keyPressEvent(QKeyEvent* event) override;
	virtual void					keyReleaseEvent(QKeyEvent* event) override;
	virtual void					paintGL() override;
};


class GLWatcherWidget : public WatcherWidget
{
public:
	GLWatcherWidget(QWidget* Parent, QGLWidget* ShareWidget, WatcherPosition Position);
	virtual ~GLWatcherWidget();

	GLWidget*						TheGLWidget;
};