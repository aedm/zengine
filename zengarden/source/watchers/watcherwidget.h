#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>

class Watcher;
class GLWidget;
class QTabWidget;

enum class WatcherPosition {
  LEFT_TAB,
  RIGHT_TAB,
  PROPERTY_PANEL
};

class WatcherWidget: public QWidget {
  friend class Watcher;

public:
  WatcherWidget(QWidget* parent, WatcherPosition position, 
                QTabWidget* tabWidget = nullptr);
  virtual ~WatcherWidget();

  /// Triggered when a new node needs to be watched (eg. pressed space in graph editor)
  Event<Node*, WatcherWidget*> onWatchNode;

  /// Triggered when a node is selected (eg. in graph editor)
  Event<Node*> onSelectNode;

  /// Triggered when the watcher needs to be removed
  FastDelegate<void(WatcherWidget*)> onWatcherDeath;

  /// Get the OpenGL widget, if any
  virtual GLWidget* GetGLWidget();

  const WatcherPosition	mPosition;
  QTabWidget* mTabWidget;

protected:
  Watcher* mWatcher;

  void SetTabLabel(const QString& text);

  /// Handle drag events
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
};


/// Boilerplate class, converts Qt virtual functions to events :(
class GLWidget: public QGLWidget {
public:
  GLWidget(QWidget* Parent, QGLWidget* ShareWidget);
  virtual ~GLWidget();

  Event<GLWidget*, QMouseEvent*> OnMouseMove;
  /// TODO: split this to left and right press/release
  Event<GLWidget*, QMouseEvent*> OnMousePress;
  Event<GLWidget*, QMouseEvent*> OnMouseRelease;
  Event<GLWidget*, QKeyEvent*> OnKeyPress;
  Event<GLWidget*, QKeyEvent*> OnKeyRelease;
  Event<GLWidget*, QWheelEvent*> OnMouseWheel;
  Event<GLWidget*> OnPaint;

protected:
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void keyReleaseEvent(QKeyEvent* event) override;
  virtual void wheelEvent(QWheelEvent * event) override;
  virtual void paintGL() override;
};


class GLWatcherWidget: public WatcherWidget {
public:
  GLWatcherWidget(QWidget* Parent, QGLWidget* ShareWidget, WatcherPosition Position,
                  QTabWidget* tabWidget = nullptr);
  virtual ~GLWatcherWidget();

  virtual GLWidget* GetGLWidget() override;
  QGLWidget* mShareWidget;

protected:
  GLWidget* mGLWidget;
};