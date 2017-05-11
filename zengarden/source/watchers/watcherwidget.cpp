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
    // Make the node release the watcher
    Node* node = mWatcher->mNode;
    if (node) node->RemoveWatcher(mWatcher.get());
    
    // Release the watcher, thereby killing its last reference
    mWatcher->mWatcherWidget = nullptr;
    mWatcher = nullptr;

    ASSERT(mWatcher.use_count() == 0);
  }
}

EventForwarderGLWidget* WatcherWidget::GetGLWidget() {
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

  mGLWidget = new EventForwarderGLWidget(this, shareWidget);
  layout->addWidget(mGLWidget);
}

GLWatcherWidget::~GLWatcherWidget() {}

EventForwarderGLWidget* GLWatcherWidget::GetGLWidget() {
  return mGLWidget;
}


EventForwarderGLWidget::EventForwarderGLWidget(QWidget* Parent, QGLWidget* ShareWidget)
  : QGLWidget(Parent, ShareWidget) {
  setAttribute(Qt::WA_OpaquePaintEvent);
}

EventForwarderGLWidget::~EventForwarderGLWidget() {}

void EventForwarderGLWidget::mouseMoveEvent(QMouseEvent* event) {
  OnMouseMove(this, event);
}

void EventForwarderGLWidget::mousePressEvent(QMouseEvent* event) {
  OnMousePress(this, event);
}

void EventForwarderGLWidget::mouseReleaseEvent(QMouseEvent* event) {
  OnMouseRelease(this, event);
}

void EventForwarderGLWidget::keyPressEvent(QKeyEvent* event) {
  OnKeyPress(this, event);
  QGLWidget::keyPressEvent(event);
}

void EventForwarderGLWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(this, event);
}

void EventForwarderGLWidget::paintGL() {
  TheDrawingAPI->OnContextSwitch();
  //TheDrawingAPI->SetViewport(0, 0, width(), height());
  OnPaint(this);
}

void EventForwarderGLWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(this, event);
}


EventForwarderWidget::EventForwarderWidget(QWidget* parent)
  : QWidget(parent) {
  setMouseTracking(true);
}

void EventForwarderWidget::paintEvent(QPaintEvent* ev) {
  mOnPaint(ev);
}

void EventForwarderWidget::mouseMoveEvent(QMouseEvent* event) {
  OnMouseMove(event);
}

void EventForwarderWidget::mousePressEvent(QMouseEvent* event) {
  OnMousePress(event);
}

void EventForwarderWidget::mouseReleaseEvent(QMouseEvent* event) {
  OnMouseRelease(event);

}
void EventForwarderWidget::keyPressEvent(QKeyEvent* event) {
  OnKeyPress(event);
}

void EventForwarderWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(event);
}

void EventForwarderWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(event);
}
