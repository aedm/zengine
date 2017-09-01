#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

class FloatSplineWatcher: public WatcherUI {
public:
  FloatSplineWatcher(Node* node);
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
  float ScreenToTime(int xPos);
  Vec2 ScreenToPoint(QPoint& pos);

  Vec2 GetStepsPerPixel();
  QPointF GetPixelsPerStep();

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

