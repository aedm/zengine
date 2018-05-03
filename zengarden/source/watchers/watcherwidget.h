#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>

class WatcherUI;
class EventForwarderGLWidget;
class QTabWidget;

enum class WatcherPosition {
  UPPER_LEFT_TAB,
  BOTTOM_LEFT_TAB,
  RIGHT_TAB,
  PROPERTY_PANEL,
  TIMELINE_PANEL,
};

class WatcherWidget: public QWidget {
  friend class WatcherUI;

public:
  WatcherWidget(QWidget* parent, shared_ptr<WatcherUI> watcher, WatcherPosition position,
                QTabWidget* tabWidget = nullptr);
  virtual ~WatcherWidget();

  /// Get the OpenGL widget, if any
  virtual EventForwarderGLWidget* GetGLWidget();

  const WatcherPosition	mPosition;
  QTabWidget* mTabWidget = nullptr;

  shared_ptr<WatcherUI> mWatcher;
protected:

  void SetTabLabel(const QString& text);

  /// Handle drag events
  virtual void dragEnterEvent(QDragEnterEvent *event);
  virtual void dropEvent(QDropEvent *event);
};


/// Boilerplate class, converts Qt virtual functions to events :(
class EventForwarderGLWidget: public QGLWidget {
public:
  EventForwarderGLWidget(QWidget* Parent, QGLWidget* ShareWidget);
  virtual ~EventForwarderGLWidget();

  Event<EventForwarderGLWidget*, QMouseEvent*> OnMouseMove;
  Event<EventForwarderGLWidget*, QMouseEvent*> OnMousePress;
  Event<EventForwarderGLWidget*, QMouseEvent*> OnMouseRelease;
  Event<EventForwarderGLWidget*, QKeyEvent*> OnKeyPress;
  Event<EventForwarderGLWidget*, QKeyEvent*> OnKeyRelease;
  Event<EventForwarderGLWidget*, QWheelEvent*> OnMouseWheel;
  Event<EventForwarderGLWidget*> OnPaint;

protected:
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void keyReleaseEvent(QKeyEvent* event) override;
  virtual void wheelEvent(QWheelEvent * event) override;
  virtual void paintGL() override;
};


/// Boilerplate class, converts Qt virtual functions to events :(
class EventForwarderWidget: public QWidget {
public:
  EventForwarderWidget(QWidget* parent);

  Event<QPaintEvent*> mOnPaint;
  Event<QMouseEvent*> OnMouseMove;
  Event<QMouseEvent*> OnMousePress;
  Event<QMouseEvent*> OnMouseRelease;
  Event<QKeyEvent*> OnKeyPress;
  Event<QKeyEvent*> OnKeyRelease;
  Event<QWheelEvent*> OnMouseWheel;

protected:
  virtual void paintEvent(QPaintEvent* ev) override;
  virtual void mouseMoveEvent(QMouseEvent* event) override;
  virtual void mousePressEvent(QMouseEvent* event) override;
  virtual void mouseReleaseEvent(QMouseEvent* event) override;
  virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void keyReleaseEvent(QKeyEvent* event) override;
  virtual void wheelEvent(QWheelEvent * event) override;
};



class GLWatcherWidget: public WatcherWidget {
public:
  GLWatcherWidget(QWidget* Parent, shared_ptr<WatcherUI> watcher, QGLWidget* ShareWidget, 
                  WatcherPosition Position, QTabWidget* tabWidget = nullptr);
  virtual ~GLWatcherWidget();

  virtual EventForwarderGLWidget* GetGLWidget() override;
  QGLWidget* mShareWidget;

protected:
  EventForwarderGLWidget* mGLWidget;
};