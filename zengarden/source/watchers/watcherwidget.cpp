#include "watcherwidget.h"
#include "watcherui.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTabWidget>
#include <utility>

WatcherWidget::WatcherWidget(QWidget* parent, shared_ptr<WatcherUi> watcher, 
  WatcherPosition position, QTabWidget* tabWidget)
  : QWidget(parent)
  , mPosition(position)
  , mTabWidget(tabWidget)
  , mWatcher(std::move(watcher))
{}

WatcherWidget::~WatcherWidget() {
  /// There are two cases: 
  /// 1. The node is killed, and it detaches the WatcherUI. The WatcherUI calls this
  ///    destructor, thereby ultimately commiting suicide.  
  /// 2. The user kills the watcher by closing it on the UI. This descturctor is called
  ///    first, and the WatcherUI is freed because this object holds the last reference
  ///    to it.
  mWatcher->mWatcherWidget = nullptr;
  if (mWatcher->GetDirectNode()) {
    mWatcher->GetDirectNode()->RemoveWatcher(mWatcher);
  }
}

EventForwarderGlWidget* WatcherWidget::GetGLWidget() {
  SHOULD_NOT_HAPPEN;
  return nullptr;
}

void WatcherWidget::SetTabLabel(const QString& text) {
  if (mTabWidget != nullptr) {
    const int index = mTabWidget->indexOf(this);
    if (index >= 0) mTabWidget->setTabText(index, text);
  }
}

void WatcherWidget::dragEnterEvent(QDragEnterEvent *event) {
  mWatcher->HandleDragEnterEvent(event);
}

void WatcherWidget::dropEvent(QDropEvent *event) {
  mWatcher->HandleDropEvent(event);
}

GLWatcherWidget::GLWatcherWidget(QWidget* parent, const shared_ptr<WatcherUi>& watcher,
  QGLWidget* shareWidget, WatcherPosition position,
  QTabWidget* tabWidget)
  : WatcherWidget(parent, watcher, position, tabWidget)
  , mShareWidget(shareWidget)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  mGLWidget = new EventForwarderGlWidget(this, shareWidget);
  layout->addWidget(mGLWidget);
}

GLWatcherWidget::~GLWatcherWidget() = default;

EventForwarderGlWidget* GLWatcherWidget::GetGLWidget() {
  return mGLWidget;
}


EventForwarderGlWidget::EventForwarderGlWidget(QWidget* Parent, QGLWidget* ShareWidget)
  : QGLWidget(Parent, ShareWidget) {
  setAttribute(Qt::WA_OpaquePaintEvent);
}

EventForwarderGlWidget::~EventForwarderGlWidget() = default;

void EventForwarderGlWidget::mouseMoveEvent(QMouseEvent* event) {
  mOnMouseMove(this, event);
}

void EventForwarderGlWidget::mousePressEvent(QMouseEvent* event) {
  mOnMousePress(this, event);
}

void EventForwarderGlWidget::mouseReleaseEvent(QMouseEvent* event) {
  mOnMouseRelease(this, event);
}

void EventForwarderGlWidget::keyPressEvent(QKeyEvent* event) {
  mOnKeyPress(this, event);
  QGLWidget::keyPressEvent(event);
}

void EventForwarderGlWidget::keyReleaseEvent(QKeyEvent* event) {
  mOnKeyRelease(this, event);
}

void EventForwarderGlWidget::paintGL() {
  OpenGL->OnContextSwitch();
  mOnPaint(this);
}

void EventForwarderGlWidget::wheelEvent(QWheelEvent * event) {
  mOnMouseWheel(this, event);
}


EventForwarderWidget::EventForwarderWidget(QWidget* parent)
  : QWidget(parent) {
  setMouseTracking(true);
}

void EventForwarderWidget::paintEvent(QPaintEvent* ev) {
  mOnPaint(ev);
}

void EventForwarderWidget::mouseMoveEvent(QMouseEvent* event) {
  mOnMouseMove(event);
}

void EventForwarderWidget::mousePressEvent(QMouseEvent* event) {
  mOnMousePress(event);
}

void EventForwarderWidget::mouseReleaseEvent(QMouseEvent* event) {
  mOnMouseRelease(event);

}
void EventForwarderWidget::keyPressEvent(QKeyEvent* event) {
  mOnKeyPress(event);
}

void EventForwarderWidget::keyReleaseEvent(QKeyEvent* event) {
  mOnKeyRelease(event);
}

void EventForwarderWidget::wheelEvent(QWheelEvent * event) {
  mOnMouseWheel(event);
}
