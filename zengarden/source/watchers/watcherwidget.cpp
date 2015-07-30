#include "watcherwidget.h"
#include "watcher.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTabWidget>

WatcherWidget::WatcherWidget(QWidget* parent, WatcherPosition position, 
                             QTabWidget* tabWidget)
  : QWidget(parent)
  , mPosition(position)
  , mWatcher(nullptr)
  , mTabWidget(tabWidget)
{}

WatcherWidget::~WatcherWidget() {
  SafeDelete(mWatcher);
}

GLWidget* WatcherWidget::GetGLWidget() {
  SHOULDNT_HAPPEN;
  return nullptr;
}

void WatcherWidget::SetTabLabel(const QString& text) {
  if (mTabWidget != nullptr) {
    int index = mTabWidget->indexOf(this);
    if (index >= 0) mTabWidget->setTabText(index, text);
  }
}

GLWatcherWidget::GLWatcherWidget(QWidget* parent, QGLWidget* shareWidget, 
                                 WatcherPosition position, QTabWidget* tabWidget)
  : WatcherWidget(parent, position, tabWidget) 
  , mShareWidget(shareWidget)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  mGLWidget = new GLWidget(this, shareWidget);
  layout->addWidget(mGLWidget);
}

GLWatcherWidget::~GLWatcherWidget() {}

GLWidget* GLWatcherWidget::GetGLWidget() {
  return mGLWidget;
}


GLWidget::GLWidget(QWidget* Parent, QGLWidget* ShareWidget)
  : QGLWidget(Parent, ShareWidget) {}

GLWidget::~GLWidget() {}

void GLWidget::mouseMoveEvent(QMouseEvent* event) {
  OnMouseMove(this, event);
}

void GLWidget::mousePressEvent(QMouseEvent* event) {
  OnMousePress(this, event);
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event) {
  OnMouseRelease(this, event);
}

void GLWidget::keyPressEvent(QKeyEvent* event) {
  OnKeyPress(this, event);
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(this, event);
}

void GLWidget::paintGL() {
  TheDrawingAPI->OnContextSwitch();
  TheDrawingAPI->SetViewport(0, 0, width(), height());
  OnPaint(this);
}

void GLWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(this, event);
}

