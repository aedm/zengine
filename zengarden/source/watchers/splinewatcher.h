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
  
protected:
  Ui::SplineEditor mUI;
  EventForwarderWidget* mSplineWidget;

  virtual void OnRedraw() override;
  virtual void OnSplineControlPointsChanged() override;
  virtual void OnTimeEdited(float time) override;

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
  SplineLayer mHoveredLayer;

  int mSelectedPointIndex = -2;
  SplineLayer mSelectedLayer = SplineLayer::NONE;

  QPoint mOriginalMousePos;

  void UpdateRangeLabels();
  void UpdateTimeEdit();
  void UpdateValueEdit();

  void SelectPoint(SplineLayer layer, int index);

  FloatSplineNode* GetSpline();

private slots:
  void DrawSpline(QPaintEvent* ev);
  void DrawSplineComponentControl(QPainter& painter, SplineLayer component);

  void RemovePoint();
  void AddPoint(SplineLayer layer);
  void ToggleLinear();

  void HandleTimeEdited();
  void HandleValueEdited();
};

typedef SplineWatcher<NodeType::FLOAT> FloatSplineWatcher;

