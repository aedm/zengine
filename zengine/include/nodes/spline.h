#pragma once

#include "valuenodes.h"
#include "timenode.h"
#include "propertiesnode.h"
#include <vector>

using namespace std;
class FloatSplineNode;

template <typename Value, typename Tangent>
struct SplineControlPoint
{
  float	mTime = 0.0f;
  Value	mValue = Value();
  Tangent	mTangentBefore = Tangent();
  Tangent mTangentAfter = Tangent();

  // Is next section is linear or curved
  bool mIsLinear = false;

  // If true, tangents are automatically calculated
  bool mIsAutoangent = true;

  // Indicates breakpoint (tangents are not continuous)
  bool mIsBreakpoint = false;
};

/// Spline abstraction to make them editable
class AbstractSpline {
public:
  virtual int GetControlPointCount() = 0;
  virtual int GetDimensionCount() = 0;

  virtual float EvaluateDimension(float time, int dimension) = 0;

  virtual float GetPointTime(int index) = 0;

  virtual float GetPointValue(int index, int dimension) = 0;
  virtual void SetPointValue(int index, int dimension, float value) = 0;

  virtual Vec2 GetPointTangentBefore(int index, int dimension) = 0;
  virtual Vec2 GetPointTangentAfter(int index, int dimension) = 0;
  virtual void SetPointTangentBefore(int index, int dimension, Vec2 tangent) = 0;
  virtual void SetPointTangentAfter(int index, int dimension, Vec2 tangent) = 0;

  /// Finds the last spline point which's time is not greater than the argument
  /// Returns -1 if there are no points before (or exactly at) the argument
  int FindPointIndexBefore(float time);
protected:
  AbstractSpline() {}

private:
  /// Cache of the last found index for performance reasons
  int mLastIndex = -1;
};

template <typename Vector>
class VectorSpline : AbstractSpline {
public:
  typedef SplineControlPoint<Vector, Vector> Point;

  Vector Evaluate(float time);

  // base
  virtual int GetControlPointCount() override;
  virtual int GetDimensionCount() override;
  virtual float EvaluateDimension(float time, int dimension) override;

  virtual float GetPointTime(int index) override;

  virtual float GetPointValue(int index, int dimension) override;
  virtual void SetPointValue(int index, int dimension, float value) override;

  virtual Vec2 GetPointTangentBefore(int index, int dimension) override;
  virtual Vec2 GetPointTangentAfter(int index, int dimension) override;
  virtual void SetPointTangentBefore(int index, int dimension, Vec2 tangent) override;
  virtual void SetPointTangentAfter(int index, int dimension, Vec2 tangent) override;

protected:
  /// Key points
  vector<Point> mPoints;

  /// Recalculates tangents for a given index
  virtual void CalculateTangent(int index) = 0;
};



