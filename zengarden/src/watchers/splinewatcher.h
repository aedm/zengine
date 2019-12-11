#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

class FloatSplineWatcher: public WatcherUi {
public:
  FloatSplineWatcher(const std::shared_ptr<Node>& node);

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;
  
protected:
  Ui::SplineEditor mUi{};
  EventForwarderWidget* mSplineWidget{};

  void OnRedraw() override;
  void OnSplineControlPointsChanged() override;
  void OnTimeEdited(float time) override;

  void HandleMouseDown(QMouseEvent* event);
  void HandleMouseUp(QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseMove(QMouseEvent* event);
  void HandleMouseWheel(QWheelEvent* event);

  QPointF ToScreenCoord(float time, float value) const;
  float ScreenToTime(int xPos) const;
  vec2 ScreenToPoint(const QPoint& pos) const;

  vec2 GetStepsPerPixel() const;
  QPointF GetPixelsPerStep() const;

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    POINT_MOVE,
    TIME_MOVE,
  };
  State mState = State::DEFAULT;

  vec2 mLeftCenterPoint = vec2(0, 0);
  vec2 mZoomLevel = vec2(0, 0);
  vec2 mOriginalPoint;

  int mHoveredPointIndex = -1;
  SplineLayer mHoveredLayer = SplineLayer::NONE;

  int mSelectedPointIndex = -2;
  SplineLayer mSelectedLayer = SplineLayer::NONE;

  QPoint mOriginalMousePos;

  void UpdateRangeLabels() const;
  void UpdateTimeEdit() const;
  void UpdateValueEdit() const;

  void SelectPoint(SplineLayer layer, int index);

  std::shared_ptr<FloatSplineNode> GetSpline() const;

private slots:
  void DrawSpline(QPaintEvent* ev) const;
  void DrawSplineComponentControl(QPainter& painter, SplineLayer layer) const;

  void RemovePoint();
  void AddPoint(SplineLayer layer);
  void ToggleLinear() const;

  void HandleTimeEdited() const;
  void HandleValueEdited() const;
};

