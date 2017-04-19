#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

template<NodeType T>
class SplineWatcher: public WatcherUI {
public:
  SplineWatcher(Node* node);
  virtual ~SplineWatcher();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

  Event<> OnAdjustTime;
  
protected:
  Ui::SplineEditor mUI;
  SplineWidget* mSplineWidget;

  virtual void OnRedraw() override;
  virtual void OnSplineControlPointsChanged() override;
  virtual void OnSplineTimeChanged() override;

  void HandleMouseDown(QMouseEvent* event);
  void HandleMouseUp(QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseMove(QMouseEvent* event);
  void HandleMouseWheel(QWheelEvent* event);

  typedef typename NodeTypes<T>::Type VType;
  QPointF ToScreenCoord(float time, float value);
  float ScreenToTime(int xPos);

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    POINT_MOVE,
    TIME_MOVE,
  };

  State mState = State::DEFAULT;

  Vec2 mXRange, mYRange;
  Vec2 mXRangeOriginal, mYRangeOriginal;
  
  float mOriginalTime;
  
  VType mOriginalValue;
  int mHoveredPointIndex = -1;
  int mSelectedPointIndex = -2;

  QPoint mOriginalMousePos;

  void UpdateRangeLabels();
  void SelectPoint(int index);

  SSpline* GetSpline();

private slots:
  void DrawSpline(QPaintEvent* ev);
  void RemovePoint();
  void AddPoint();
  void ToggleLinear();
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
  //virtual void keyPressEvent(QKeyEvent* event) override;
  virtual void keyReleaseEvent(QKeyEvent* event) override;
  virtual void wheelEvent(QWheelEvent * event) override;
};
