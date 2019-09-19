#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

class FloatSplineWatcher: public WatcherUI {
public:
  FloatSplineWatcher(const shared_ptr<Node>& node);
  virtual ~FloatSplineWatcher();

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

  QPointF ToScreenCoord(float time, float value);
  float ScreenToTime(int xPos) const;
  Vec2 ScreenToPoint(QPoint& pos) const;

  Vec2 GetStepsPerPixel() const;
  QPointF GetPixelsPerStep() const;

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    POINT_MOVE,
    TIME_MOVE,
  };
  State mState = State::DEFAULT;

  Vec2 mLeftCenterPoint = Vec2(0, 0);
  Vec2 mZoomLevel = Vec2(0, 0);
  Vec2 mOriginalPoint;

  int mHoveredPointIndex = -1;
  SplineLayer mHoveredLayer;

  int mSelectedPointIndex = -2;
  SplineLayer mSelectedLayer = SplineLayer::NONE;

  QPoint mOriginalMousePos;

  void UpdateRangeLabels() const;
  void UpdateTimeEdit() const;
  void UpdateValueEdit() const;

  void SelectPoint(SplineLayer layer, int index);

  shared_ptr<FloatSplineNode> GetSpline() const;

private slots:
  void DrawSpline(QPaintEvent* ev);
  void DrawSplineComponentControl(QPainter& painter, SplineLayer component);

  void RemovePoint();
  void AddPoint(SplineLayer layer);
  void ToggleLinear() const;

  void HandleTimeEdited() const;
  void HandleValueEdited() const;
};

