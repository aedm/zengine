#pragma once

#include "valuenodes.h"
#include "timenode.h"
#include "propertiesnode.h"
#include <vector>

using namespace std;
template <ValueType> class SplineNode;

const float Epsilon = 0.0001f;

/// A spline point
template <typename T>
struct SplinePoint
{
  float	mTime = 0.0f;
  T mValue = T();
  T mTangentBefore = T();
  T mTangentAfter = T();

  // Is next section is linear or curved
  bool mIsLinear;

  // If true, tangents are automatically calculated
  bool mIsAutoangent;

  // Indicates breakpoint (tangents are not continuous)
  bool mIsBreakpoint;

  void SetValue(float time, T value) {
    this->mTime = time;
    this->mValue = value;
  };
};


/// A simple spline consisting of a set of points. Acts as a component of spline nodes 
/// which have several layers of basic splines. In each layer there is one basic spline.
template <typename T>
class Spline {
  friend class SplineNode<ValueType::FLOAT>;
  friend class SplineNode<ValueType::VEC3>;

public:
  /// Returns the points of the spline
  const vector<SplinePoint<T>>& GetPoints() {
    return mPoints;
  };

  /// Evaluates spline at a given time
  T Get(float time) {
    int index = FindPointIndexBefore(time);

    SplinePoint<T>* before = index >= 0 ? &mPoints[index] : nullptr;
    SplinePoint<T>* after =
      index < int(mPoints.size()) - 1 ? &mPoints[index + 1] : nullptr;

    if (before && after)
      if (before->mIsLinear) {
        float dt = after->mTime - before->mTime;
        if (dt <= Epsilon) return before->mValue;
        return (after->mValue * (time - before->mTime) +
          before->mValue * (after->mTime - time)) / dt;
      }
      else {
        float dt = after->mTime - before->mTime;
        float ft = (time - before->mTime) / dt;

        T ea = before->mValue;
        T eb = before->mTangentAfter * dt;
        T ec = (after->mValue - before->mValue) * 3.0f -
          (before->mTangentAfter * 2.0f + after->mTangentBefore)*dt;
        T ed = (after->mValue - before->mValue) * -2.0f +
          (before->mTangentAfter + after->mTangentBefore) * dt;

        return ea + eb * ft + ec * ft*ft + ed * ft*ft*ft;
      }
    return before ? before->mValue : after ? after->mValue : T();
  };

private:
  /// Disallow direct instantiation. Only SplineNodes can create splines.
  Spline() {}

  /// Key points
  vector<SplinePoint<T>> mPoints;

  /// Finds the last spline point which's time is not greater than the argument
  /// Returns -1 if there are no points before (or exactly at) the argument
  int FindPointIndexBefore(float time) {
    int i = mLastIndex;
    if (i >= int(mPoints.size())) i = -1;
    while (i < int(mPoints.size()) - 1 && mPoints[i + 1].mTime <= time) i++;
    while (i >= 0 && mPoints[i].mTime > time) i--;
    mLastIndex = i;
    return i;
  };

  /// Recalculates tangents for a given index
  void CalculateTangent(int index) {
    if (index < 0 || index >= int(mPoints.size())) return;
    SplinePoint<T>& point = mPoints[index];
    if (point.mIsAutoangent) {
      T ym = T();
      if (index > 0 && index < mPoints.size() - 1) {
        SplinePoint<T>& prev = mPoints[index - 1];
        SplinePoint<T>& next = mPoints[index + 1];
        float xm = next.mTime - prev.mTime;
        ym = next.mValue - prev.mValue;
        if (xm > Epsilon) ym /= xm;
      }
      point.mTangentAfter = point.mTangentBefore = ym;
    }
  };

  /// Adds a point to the spline, returns its index
  int AddPoint(float time, T value) {
    int i = 0;
    while (i < mPoints.size() && mPoints[i].mTime <= time) i++;

    SplinePoint<T> tmp;
    tmp.mTime = time;
    tmp.mValue = value;
    tmp.mIsAutoangent = true;
    tmp.mIsBreakpoint = false;
    tmp.mIsLinear = i > 0 ? mPoints[i - 1].mIsLinear : false;

    mPoints.insert(mPoints.begin() + i, tmp);

    CalculateTangent(i - 1);
    CalculateTangent(i);
    CalculateTangent(i + 1);

    return i;
  };

  /// Sets a point's time and value
  void SetPointValue(int index, float time, T value) {
    if (index < 0 || index >= int(mPoints.size())) return;
    SplinePoint<T>& point = mPoints[index];

    if (index > 0 && mPoints[index - 1].mTime > time) time = mPoints[index - 1].mTime;
    if (index < mPoints.size() - 1 && mPoints[index + 1].mTime < time) {
      time = mPoints[index + 1].mTime;
    }

    point.mTime = time;
    point.mValue = value;
    CalculateTangent(index - 1);
    CalculateTangent(index);
    CalculateTangent(index + 1);
  };

  void SetAutotangent(int index, bool autotangent) {
    if (index < 0 || index >= int(mPoints.size())) return;
    mPoints[index].mIsAutoangent = autotangent;
    CalculateTangent(index);
  };

  void SetLinear(int index, bool linear) {
    if (index < 0 || index >= int(mPoints.size())) return;
    mPoints[index].mIsLinear = linear;
  };

  void SetBreakpoint(int index, bool breakpoint) {
    if (index < 0 || index >= int(mPoints.size())) return;
    mPoints[index].mIsBreakpoint = breakpoint;
    CalculateTangent(index);
  };

  void RemovePoint(int index) {
    if (index < 0 || index >= int(mPoints.size())) return;
    mPoints.erase(mPoints.begin() + index);
    CalculateTangent(index - 1);
    CalculateTangent(index);
    CalculateTangent(index + 1);
  }

  /// Cache of the last found index for performance reasons
  int mLastIndex = -1;
};


/// Spline layers
enum class SplineLayer {
  /// Base value layer
  BASE,

  /// Modifier layers
  NOISE,

  BEAT_SPIKE_FREQUENCY,
  BEAT_SPIKE_INTENSITY,

  BEAT_QUANTIZER,

  /// Number of layers
  COUNT,
  NONE = COUNT,
};

template <ValueType T>
class SplineNode : public ValueNode<T>
{
  friend class JSONSerializer;

  typedef typename ValueTypes<T>::Type VType;

public:
  SplineNode();

  virtual ~SplineNode() {
    mSceneTimeNode->Dispose();
  };

  /// Returns spline value at current scene time
  virtual const VType& Get() override {
    Update();
    return currentValue;
  };

  /// Returns spline components value at a given time
  VType GetValue(float time);

  /// Noise component
  FloatSlot mNoiseEnabled;
  FloatSlot mNoiseVelocity;

  /// Beat spike component
  FloatSlot mBeatSpikeEnabled;
  FloatSlot mBeatSpikeLength;
  FloatSlot mBeatSpikeEasing;
  FloatSlot mBeatQuantizerFrequency;

  /// Adds a point to the spline, returns its index
  int AddBasePoint(float time, VType value) {
    int index = mBaseLayer.AddPoint(time, value);
    InvalidateCurrentValue();
    return index;
  };

  int AddLayerPoint(SplineLayer layer, float time, float value) {
    int index = GetComponent(layer)->AddPoint(time, value);
    InvalidateCurrentValue();
    return index;
  };

  /// Sets a point's time and value
  void SetBaseValue(int index, float time, VType value) {
    mBaseLayer.SetPointValue(index, time, value);
    InvalidateCurrentValue();
  };

  void SetLayerValue(SplineLayer layer, int index, float time, float value) {
    GetComponent(layer)->SetPointValue(index, time, value);
    InvalidateCurrentValue();
  };

  /// Removes a point at 'index'
  void RemovePoint(SplineLayer layer, int index) {
    if (layer == SplineLayer::BASE) mBaseLayer.RemovePoint(index);
    else GetComponent(layer)->RemovePoint(index);
    InvalidateCurrentValue();
  };;

  /// Sets whether the spline is C1 continuous at the Nth point, or the 
  /// tangents are independent (C0)
  void SetBreakpoint(SplineLayer layer, int index, bool breakpoint) {
    if (layer == SplineLayer::BASE) mBaseLayer.SetBreakpoint(index, breakpoint);
    else GetComponent(layer)->SetBreakpoint(index, breakpoint);
    InvalidateCurrentValue();
  };

  /// Sets whether the segment after a given point is linear or not
  void SetLinear(SplineLayer layer, int index, bool linear) {
    if (layer == SplineLayer::BASE) mBaseLayer.SetLinear(index, linear);
    else GetComponent(layer)->SetLinear(index, linear);
    InvalidateCurrentValue();
  };

  /// Sets whether tangents should be automatically calculated for a given point
  void SetAutotangent(SplineLayer layer, int index, bool autotangent) {
    if (layer == SplineLayer::BASE) mBaseLayer.SetAutotangent(index, autotangent);
    else GetComponent(layer)->SetAutotangent(index, autotangent);
    InvalidateCurrentValue();
  };

  /// Returns the modifier component in a certain layer
  Spline<float>* GetComponent(SplineLayer layer) {
    switch (layer) {
    case SplineLayer::NOISE:
      return &mNoiseLayer;
    case SplineLayer::BEAT_SPIKE_FREQUENCY:
      return &mBeatSpikeFrequencyLayer;
    case SplineLayer::BEAT_SPIKE_INTENSITY:
      return &mBeatSpikeIntensityLayer;
    case SplineLayer::BEAT_QUANTIZER:
      return &mBeatQuantizerLayer;
      break;
    default:
      SHOULD_NOT_HAPPEN;
      return nullptr;
    }
  };

  /// Scene time node
  shared_ptr<SceneTimeNode> mSceneTimeNode;

  /// Time slot, connected to mSceneTimeNode
  FloatSlot mTimeSlot;

  /// Base spline
  Spline<VType> mBaseLayer;

protected:
  virtual void HandleMessage(Message* message) override {
    switch (message->mType) {
    case MessageType::VALUE_CHANGED:
      InvalidateCurrentValue();
      NotifyWatchers(&Watcher::OnTimeEdited, mSceneTimeNode->Get());
      break;
    default:
      break;
    }
  };

  /// Computer layer values
  float GetNoiseValue(float time) {
    if (mNoiseEnabled.Get() < 0.5f) return 0.0f;
    float noiseVelocity = mNoiseVelocity.Get();
    float noiseRatio = mNoiseLayer.Get(time) * 0.33f;
    float t = time * noiseVelocity;
    return noiseRatio * (sinf(t * 0.67f) + cosf(t * 2.43f) + cosf(t * 3.81f + 0.5f));
  };

  float GetBeatSpikeValue(float time) {
    if (mBeatSpikeEnabled.Get() < 0.5f) return 0.0f;

    float freq = mBeatSpikeFrequencyLayer.Get(time);
    if (freq < Epsilon) return 0.0f;

    float length = mBeatSpikeLength.Get();
    if (length < Epsilon) return 0.0f;

    float subBeat = time - freq * floorf(time / freq);
    if (subBeat > length) return 0.0f;

    float intensity = mBeatSpikeIntensityLayer.Get(time);
    float easing = mBeatSpikeEasing.Get();

    float t = 1.0f - subBeat / length;
    return intensity * powf(t, easing);
  };

  float GetBeatQuantizerValue(float time) {
    float freq = mBeatQuantizerFrequency.Get();
    if (freq < Epsilon) return 0.0f;

    float t = freq * floorf(time / freq);
    return mBeatQuantizerLayer.Get(t);
  };

  //float EvaluateLinearSpline(vector<SplinePoint>& points, float time);

  /// Control points of spline
  Spline<float> mNoiseLayer;
  Spline<float> mBeatSpikeIntensityLayer;
  Spline<float> mBeatSpikeFrequencyLayer;
  Spline<float> mBeatQuantizerLayer;

  /// Current value
  VType currentValue;

  void InvalidateCurrentValue() {
    if (mIsUpToDate) {
      mIsUpToDate = false;
      SendMsg(MessageType::VALUE_CHANGED);
    }
    NotifyWatchers(&Watcher::OnRedraw);
  };

  virtual void Operate() {
    currentValue = GetValue(mTimeSlot.Get());
  };
};


typedef SplineNode<ValueType::FLOAT> FloatSplineNode;
typedef SplineNode<ValueType::VEC3> Vec3SplineNode;
