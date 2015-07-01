#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>

class Watcher;
class GLWidget;

enum class WatcherPosition {
  LEFT_TAB,
  RIGHT_TAB,
  PROPERTY_PANEL
};

class WatcherWidget: public QWidget {
  friend class Watcher;

public:
  WatcherWidget(QWidget* parent, WatcherPosition position);
  virtual ~WatcherWidget();

  /// Triggered when a new node needs to be watched (eg. pressed space in graph editor)
  Event<Node*, WatcherWidget*> onWatchNode;

  /// Triggered when a node is selected (eg. in graph editor)
  Event<Node*> onSelectNode;
  
  /// Get the OpenGL widget, if any
  virtual GLWidget* GetGLWidget();

  const WatcherPosition	mPosition;

protected:
  Watcher* mWatcher;
};


/// Boilerplate class, converts Qt virtual functions to events :(
class GLWidget: public QGLWidget {
public:
  GLWidget(QWidget* Parent, QGLWidget* ShareWidget);
  virtual ~GLWidget();

  Event<GLWidget*, QMouseEvent*> OnMouseMove;
  Event<GLWidget*, QMouseEvent*> OnMousePress;
  Event<GLWidget*, QMouseEvent*> OnMouseRelease;
  Event<GLWidget*, QKeyEvent*> OnKeyPress;
  Event<GLWidget*, QKeyEvent*> OnKeyRelease;
  Event<GLWidget*> OnPaint;

protected:
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void keyReleaseEvent(QKeyEvent* event) override;
  virtual void paintGL() override;
};


class GLWatcherWidget: public WatcherWidget {
public:
  GLWatcherWidget(QWidget* Parent, QGLWidget* ShareWidget, WatcherPosition Position);
  virtual ~GLWatcherWidget();

  virtual GLWidget* GetGLWidget() override;

protected:
  GLWidget* mGLWidget;
};