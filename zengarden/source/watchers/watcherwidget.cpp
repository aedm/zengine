#include "watcherwidget.h"
#include "watcherui.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTabWidget>

WatcherWidget::WatcherWidget(QWidget* parent, shared_ptr<WatcherUI> watcher, WatcherPosition position,
                             QTabWidget* tabWidget)
  : QWidget(parent)
  , mPosition(position)
  , mTabWidget(tabWidget)
  , mWatcher(watcher)
{}

WatcherWidget::~WatcherWidget() {
  if (mWatcher) {
    mWatcher->mWatcherWidget = nullptr;
    mWatcher->Unwatch();
  }
  //ASSERT(mWatcher.use_count() == 1);
}

GLWidget* WatcherWidget::GetGLWidget() {
  SHOULD_NOT_HAPPEN;
  return nullptr;
}

void WatcherWidget::SetTabLabel(const QString& text) {
  if (mTabWidget != nullptr) {
    int index = mTabWidget->indexOf(this);
    if (index >= 0) mTabWidget->setTabText(index, text);
  }
}

void WatcherWidget::dragEnterEvent(QDragEnterEvent *event) {
  mWatcher->HandleDragEnterEvent(event);
}

void WatcherWidget::dropEvent(QDropEvent *event) {
  mWatcher->HandleDropEvent(event);
}

GLWatcherWidget::GLWatcherWidget(QWidget* parent, shared_ptr<WatcherUI> watcher, 
                                 QGLWidget* shareWidget, WatcherPosition position, 
                                 QTabWidget* tabWidget)
  : WatcherWidget(parent, watcher, position, tabWidget)
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
  : QGLWidget(Parent, ShareWidget) {
  setAttribute(Qt::WA_OpaquePaintEvent);
}

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
  QGLWidget::keyPressEvent(event);
}

void GLWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(this, event);
}

void GLWidget::paintGL() {
  TheDrawingAPI->OnContextSwitch();
  //TheDrawingAPI->SetViewport(0, 0, width(), height());
  OnPaint(this);
}

void GLWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(this, event);
}


EventWidget::EventWidget(QWidget* parent)
  : QWidget(parent) {
  setMouseTracking(true);
}

void EventWidget::paintEvent(QPaintEvent* ev) {
  mOnPaint(ev);
}

void EventWidget::mouseMoveEvent(QMouseEvent* event) {
  OnMouseMove(event);
}

void EventWidget::mousePressEvent(QMouseEvent* event) {
  OnMousePress(event);
}

void EventWidget::mouseReleaseEvent(QMouseEvent* event) {
  OnMouseRelease(event);

}
void EventWidget::keyPressEvent(QKeyEvent* event) {
  OnKeyPress(event);
}

void EventWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(event);
}

void EventWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(event);
}
