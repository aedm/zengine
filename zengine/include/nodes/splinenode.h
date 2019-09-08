#pragma once

#include "valuenodes.h"
#include "timenode.h"
#include "propertiesnode.h"
#include <vector>

using namespace std;
class FloatSplineNode;

struct SplinePoint
{
	SplinePoint();

	float	mTime;
	float	mValue;
	float	mTangentBefore, mTangentAfter;

  // Is next section is linear or curved
  bool mIsLinear;

  // If true, tangents are automatically calculated
	bool mIsAutoangent;					

  // Indicates breakpoint (tangents are not continuous)
	bool mIsBreakpoint;						

	void SetValue(float time, float value);
};


/// A simple spline component consisting of one set of points. Acts as a component 
/// to spline nodes which have several layers of basic splines. In each layer there is
/// one spline component.
class SplineComponent {
  friend class FloatSplineNode;

public:
  /// Having a virtual destructor makes sure the class has RTTI information
  virtual ~SplineComponent() {}

  /// Returns the points of the spline
  const vector<SplinePoint>& GetPoints();

protected:
  /// Disallow direct instantiation
  SplineComponent() {}

  /// Finds the last spline point which's time is not greater than the argument
  /// Returns -1 if there are no points before (or exactly at) the argument
  int FindPointIndexBefore(float time);

  /// Key points
  vector<SplinePoint> mPoints;

  /// Recalculates tangents for a given index
  virtual void CalculateTangent(int index) = 0;

private:
  /// Cache of the last found index for performance reasons
  int mLastIndex = -1;
};


/// A simple float spline component
class SplineFloatComponent: public SplineComponent {
  friend class FloatSplineNode;

public:
  SplineFloatComponent() {}
  virtual ~SplineFloatComponent() {}

  float Get(float time);

protected:
  /// Adds a point to the spline, returns its index
  int AddPoint(float time, float value);

  /// Sets a point's time and value
  void SetPointValue(int index, float time, float value);

  /// Calculates tangents of the Nth control point
  virtual void CalculateTangent(int index) override;
};


/// Spline layers
enum class SplineLayer {
  /// Base layer
  BASE,

  /// Additional modifier layers
  NOISE,

  BEAT_SPIKE_FREQUENCY,
  BEAT_SPIKE_INTENSITY,

  BEAT_QUANTIZER,

  /// Number of layers
  COUNT,
  NONE = COUNT,
};


class FloatSplineNode: public ValueNode<float>
{
public:
	FloatSplineNode();
  virtual ~FloatSplineNode();

  /// Returns spline value at current scene time
  virtual const float& Get() override;

  /// Returns spline components value at a given time
  float GetValue(float time);

  /// Noise component
  FloatSlot mNoiseEnabled;
  FloatSlot mNoiseVelocity;

  /// Beat spike component
  FloatSlot mBeatSpikeEnabled;
  FloatSlot mBeatSpikeLength;
  FloatSlot mBeatSpikeEasing;
  FloatSlot mBeatQuantizerFrequency;

  /// Adds a point to the spline, returns its index
  int AddPoint(SplineLayer layer, float time, float value);

  /// Sets a point's time and value
  void SetPointValue(SplineLayer layer, int index, float time, float value);

  /// Removes a point at 'index'
  void RemovePoint(SplineLayer layer, int index);

  /// Sets whether the spline is C1 continuous at the Nth point, or the 
  /// tangents are independent (C0)
  void SetBreakpoint(SplineLayer layer, int index, bool breakpoint);

  /// Sets whether the segment after a given point is linear or not
  void SetLinear(SplineLayer layer, int index, bool linear);

  /// Sets whether tangents should be automatically calculated for a given point
  void SetAutotangent(SplineLayer layer, int index, bool autotangent);

  void SetBaseOffset(float baseOffset);
  void AddBasePointWithOffset();

  /// Returns the component in a certain layer
  SplineFloatComponent* GetComponent(SplineLayer layer);

  /// Scene time node
  shared_ptr<SceneTimeNode> mSceneTimeNode;

  /// Time slot, connected to mSceneTimeNode
  FloatSlot mTimeSlot;

protected:
  virtual void HandleMessage(Message* message) override;

  /// Computer layer values
  float GetNoiseValue(float time);
  float GetBeatSpikeValue(float time);
  float GetBeatQuantizerValue(float time);

  float EvaluateLinearSpline(vector<SplinePoint>& points, float time);

  /// Control points of spline
  SplineFloatComponent mBaseLayer;
  SplineFloatComponent mNoiseLayer;
  SplineFloatComponent mBeatSpikeIntensityLayer;
  SplineFloatComponent mBeatSpikeFrequencyLayer;
  SplineFloatComponent mBeatQuantizerLayer;

  /// Current value
  float currentValue = 0.0f;
  float mCurrentValuePlusBaseOffset = 0.0f;

  /// User-set base offset. For temporary overrides, experimenting
  float mBaseOffset = 0.0f;

  void InvalidateCurrentValue();
  virtual void Operate();
};


