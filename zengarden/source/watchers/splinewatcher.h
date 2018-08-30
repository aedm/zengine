#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_splineeditor.h"

#include <zengine.h>

class SplineWidget;

class FloatSplineWatcher: public WatcherUI {
public:
  FloatSplineWatcher(const shared_ptr<Node>& node);

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

  struct PointSelection {
    PointSelection(SplineLayer layer, int index, int component);
    SplineLayer mLayer;
    int mIndex;
    int mComponent;
    bool operator==(const PointSelection& op);
    bool isValid();
    void inValidate();
    static PointSelection None();
  };

  PointSelection mHoveredPoint = PointSelection::None();
  PointSelection mSelectedPoint = PointSelection::None();

  QPoint mOriginalMousePos;

  void UpdateRangeLabels();
  void UpdateTimeEdit();
  void UpdateValueEdit();

  void SelectPoint(const PointSelection& pointSelection, bool force = false);

  shared_ptr<FloatSplineNode> GetSpline();

private slots:
  void DrawSpline(QPaintEvent* ev);

  /// Draw a float spline
  void DrawFloatSpline(QPainter& painter, Spline<float>* spline, SplineLayer layer);

  template<typename T>
  void DrawBaseSpline(Spline<T>* spline, int layer);

  /// Check mouse over spline component
  template<typename T>
  bool CheckMouseOverLayer(Spline<T>* component, SplineLayer layer, QPoint mouse, 
    int componentCount);

  /// Check mouse over spline point
  template<ValueType V, int ComponentCount>
  void FindHover(shared_ptr<SplineNode<V>> splineNode, QPoint mouse);


  void DrawSplineComponentControl(QPainter& painter, SplineLayer component);

  void RemovePoint();

  /// Add new point to a certain layer
  template<ValueType V, int ComponentCount>
  void AddPoint(shared_ptr<SplineNode<V>> splineNode, SplineLayer layer);
  void AddPoint(SplineLayer layer);

  /// Return whether a spline point is linear
  template<ValueType V, int ComponentCount>
  bool IsSplinePointLinear(shared_ptr<SplineNode<V>> splineNode, SplineLayer layer, 
    int index);
  bool IsSplinePointLinear(SplineLayer layer, int index);

  /// Toggles linear flag
  template<ValueType V, int ComponentCount>
  void TogglePointLinear(shared_ptr<SplineNode<V>> splineNode, SplineLayer layer,
    int index);
  void TogglePointLinear(SplineLayer layer, int index);

  void ToggleLinear();

  /// Returns the time component of a control point
  template<ValueType V, int ComponentCount>
  float GetPointTime(shared_ptr<SplineNode<V>> splineNode, SplineLayer layer,
    int index);
  float GetPointTime(SplineLayer layer, int index);

  template<ValueType V, int ComponentCount>
  float GetPointValue(shared_ptr<SplineNode<V>> splineNode, PointSelection& selection);
  float GetPointValue(PointSelection& selection);

  void HandleTimeEdited();
  void HandleValueEdited();

  vector<QPointF> mDrawPoints;
  vector<float[4]> mSplinePoints;
};

