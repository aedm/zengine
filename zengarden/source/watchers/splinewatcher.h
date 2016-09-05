#pragma once

#include "watcher.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

template<NodeType T>
class SplineWatcher: public Watcher {
public:
  SplineWatcher(Node* node, WatcherWidget* watcherWidget);

protected:
  Ui::SplineEditor mUI;
  SplineWidget* mSplineWidget;

private slots:
  void DrawSpline(QPaintEvent* ev);

  void HandleMouseDown(QMouseEvent* event);
  void HandleMouseUp(QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseMove(QMouseEvent* event);
  void HandleMouseWheel(QWheelEvent* event);

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    POINT_MOVE,
  };

  State mState = State::DEFAULT;

  Vec2 mXRange, mYRange;
  Vec2 mXRangeOriginal, mYRangeOriginal;
  
  float mOriginalTime;
  
  typedef typename NodeTypes<T>::Type VType;
  VType mOriginalValue;
  int mHoveredPointIndex = -1;

  QPoint mOriginalMousePos;

  void UpdateRangeLabels();
};


typedef SplineWatcher<NodeType::FLOAT> FloatSplineWatcher;

class SplineWidget: public QWidget {
public:
  SplineWidget(QWidget* parent);

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
