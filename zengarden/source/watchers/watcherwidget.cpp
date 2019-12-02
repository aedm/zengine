#include "watcherwidget.h"
#include "watcherui.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTabWidget>
#include <utility>

WatcherWidget::WatcherWidget(QWidget* parent, std::shared_ptr<WatcherUi> watcher, 
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

QGLWidget* WatcherWidget::GetGlWidget() {
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

/// Qt requires us to subclass from QGLWidget to add a custom paint handler.
/// This boilerplate class forwards paintGL() event to the watcher widget. 
class GlPaintForwarderWidget : public QGLWidget {
public:
  GlPaintForwarderWidget(GLWatcherWidget* parent, QGLWidget* shareWidget):
    QGLWidget(parent, shareWidget),
    mParent(parent) {}
  GLWatcherWidget* mParent;
protected:
  void paintGL() override {
    mParent->HandleGlPaint();
  };
};


GLWatcherWidget::GLWatcherWidget(QWidget* parent, const std::shared_ptr<WatcherUi>& watcher,
  QGLWidget* shareWidget, WatcherPosition position,
  QTabWidget* tabWidget)
  : WatcherWidget(parent, watcher, position, tabWidget)
  //, mShareWidget(shareWidget)
{
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  //mGlWidget = new EventForwarderGlWidget(this, shareWidget);
  auto glWidget = new GlPaintForwarderWidget(this, shareWidget);
  layout->addWidget(glWidget);
  this->mGlWidget = glWidget;
}

GLWatcherWidget::~GLWatcherWidget() = default;

QGLWidget* GLWatcherWidget::GetGlWidget() {
  return mGlWidget;
}
//
//
//EventForwarderGlWidget::EventForwarderGlWidget(QWidget* Parent, QGLWidget* ShareWidget)
//  : QGLWidget(Parent, ShareWidget) {
//  setAttribute(Qt::WA_OpaquePaintEvent);
//}
//
//EventForwarderGlWidget::~EventForwarderGlWidget() = default;
//
//void EventForwarderGlWidget::mouseMoveEvent(QMouseEvent* event) {
//  mOnMouseMove(this, event);
//}
//
//void EventForwarderGlWidget::mousePressEvent(QMouseEvent* event) {
//  mOnMousePress(this, event);
//}
//
//void EventForwarderGlWidget::mouseReleaseEvent(QMouseEvent* event) {
//  mOnMouseRelease(this, event);
//}
//
//void EventForwarderGlWidget::keyPressEvent(QKeyEvent* event) {
//  mOnKeyPress(this, event);
//  QGLWidget::keyPressEvent(event);
//}
//
//void EventForwarderGlWidget::keyReleaseEvent(QKeyEvent* event) {
//  mOnKeyRelease(this, event);
//}
//
//void EventForwarderGlWidget::paintGL() {
//  OpenGL->OnContextSwitch();
//  mOnPaint(this);
//}
//
//void EventForwarderGlWidget::wheelEvent(QWheelEvent * event) {
//  mOnMouseWheel(this, event);
//}
//
//
//EventForwarderWidget::EventForwarderWidget(QWidget* parent)
//  : QWidget(parent) {
//  setMouseTracking(true);
//}

void WatcherWidget::paintEvent(QPaintEvent* ev) {
  mOnPaint();
}

void WatcherWidget::mouseMoveEvent(QMouseEvent* event) {
  mOnMouseMove(event);
}

void WatcherWidget::mousePressEvent(QMouseEvent* event) {
  mOnMousePress(event);
}

void WatcherWidget::mouseReleaseEvent(QMouseEvent* event) {
  mOnMouseRelease(event);

}
void WatcherWidget::keyPressEvent(QKeyEvent* event) {
  mOnKeyPress(event);
}

void WatcherWidget::keyReleaseEvent(QKeyEvent* event) {
  mOnKeyRelease(event);
}

void WatcherWidget::wheelEvent(QWheelEvent * event) {
  mOnMouseWheel(event);
}
