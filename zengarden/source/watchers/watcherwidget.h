#pragma once

#include <zengine.h>
#include <QtOpenGL/QGLWidget>

class WatcherUi;
class EventForwarderGlWidget;
class QTabWidget;

enum class WatcherPosition {
  UPPER_LEFT_TAB,
  BOTTOM_LEFT_TAB,
  RIGHT_TAB,
  PROPERTY_PANEL,
  TIMELINE_PANEL,
};

class WatcherWidget: public QWidget {
  friend class WatcherUi;

public:
  WatcherWidget(QWidget* parent, shared_ptr<WatcherUi> watcher, WatcherPosition position,
                QTabWidget* tabWidget = nullptr);
  virtual ~WatcherWidget();

  /// Get the OpenGL widget, if any
  virtual EventForwarderGlWidget* GetGLWidget();

  const WatcherPosition	mPosition;
  QTabWidget* mTabWidget = nullptr;

  shared_ptr<WatcherUi> mWatcher;
protected:

  void SetTabLabel(const QString& text);

  /// Handle drag events
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
};


/// Boilerplate class, converts Qt virtual functions to events :(
class EventForwarderGlWidget: public QGLWidget {
public:
  EventForwarderGlWidget(QWidget* Parent, QGLWidget* ShareWidget);
  virtual ~EventForwarderGlWidget();

  Event<EventForwarderGlWidget*, QMouseEvent*> mOnMouseMove;
  Event<EventForwarderGlWidget*, QMouseEvent*> mOnMousePress;
  Event<EventForwarderGlWidget*, QMouseEvent*> mOnMouseRelease;
  Event<EventForwarderGlWidget*, QKeyEvent*> mOnKeyPress;
  Event<EventForwarderGlWidget*, QKeyEvent*> mOnKeyRelease;
  Event<EventForwarderGlWidget*, QWheelEvent*> mOnMouseWheel;
  Event<EventForwarderGlWidget*> mOnPaint;

protected:
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent * event) override;
  void paintGL() override;
};


/// Boilerplate class, converts Qt virtual functions to events :(
class EventForwarderWidget: public QWidget {
public:
  EventForwarderWidget(QWidget* parent);

  Event<QPaintEvent*> mOnPaint;
  Event<QMouseEvent*> mOnMouseMove;
  Event<QMouseEvent*> mOnMousePress;
  Event<QMouseEvent*> mOnMouseRelease;
  Event<QKeyEvent*> mOnKeyPress;
  Event<QKeyEvent*> mOnKeyRelease;
  Event<QWheelEvent*> mOnMouseWheel;

protected:
  void paintEvent(QPaintEvent* ev) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent * event) override;
};



class GLWatcherWidget: public WatcherWidget {
public:
  GLWatcherWidget(QWidget* parent, const shared_ptr<WatcherUi>& watcher, QGLWidget* shareWidget, 
                  WatcherPosition position, QTabWidget* tabWidget = nullptr);
  virtual ~GLWatcherWidget();

  EventForwarderGlWidget* GetGLWidget() override;
  QGLWidget* mShareWidget;

protected:
  EventForwarderGlWidget* mGLWidget;
};