// general/SSpline.cpp

#include <include/nodes/splinenode.h>
#include <include/nodes/scenenode.h>
#include <math.h>

REGISTER_NODECLASS(FloatSplineNode, "Float Spline");
REGISTER_NODECLASS(Vec3SplineNode, "Vec3 Spline");

static SharedString TimeSlotName = make_shared<string>("Time");
static SharedString PropertiesSlotName = make_shared<string>("Properties");
static SharedString NoiseEnabledSlotName = make_shared<string>("Noise enabled");
static SharedString NoiseVelocitySlotName = make_shared<string>("Noise velocity");
static SharedString BeatSpikeEnabledSlotName = make_shared<string>("Beat spike enabled");
static SharedString BeatSpikeLengthSlotName = make_shared<string>("Beat spike length");
static SharedString BeatSpikeEasingSlotName = make_shared<string>("Beat spike easing");
static SharedString BeatQuantizerFrequencySlotName = make_shared<string>("Quantizer freq");

template <>
SplineNode<ValueType::FLOAT>::SplineNode()
  : ValueNode<ValueType::FLOAT>()
  , mTimeSlot(this, TimeSlotName, false, false, false)
  , mNoiseEnabled(this, NoiseEnabledSlotName)
  , mNoiseVelocity(this, NoiseVelocitySlotName, false, true, true, 0.0f, 30.0f) 
  , mBeatSpikeEnabled(this, BeatSpikeEnabledSlotName)
  , mBeatSpikeLength(this, BeatSpikeLengthSlotName)
  , mBeatSpikeEasing(this, BeatSpikeEasingSlotName)
  , mBeatQuantizerFrequency(this, BeatQuantizerFrequencySlotName)
  , mSceneTimeNode(make_shared<SceneTimeNode>())
{
  mTimeSlot.Connect(mSceneTimeNode);
  mNoiseVelocity.SetDefaultValue(20.0f);
  mBeatSpikeEasing.SetDefaultValue(1.0f);
  mBeatSpikeLength.SetDefaultValue(0.5f);
}

template <>
SplineNode<ValueType::VEC3>::SplineNode()
  : ValueNode<ValueType::VEC3>()
  , mTimeSlot(this, TimeSlotName, false, false, false)
  , mNoiseEnabled(this, NoiseEnabledSlotName)
  , mNoiseVelocity(this, NoiseVelocitySlotName, false, true, true, 0.0f, 30.0f)
  , mBeatSpikeEnabled(this, BeatSpikeEnabledSlotName)
  , mBeatSpikeLength(this, BeatSpikeLengthSlotName)
  , mBeatSpikeEasing(this, BeatSpikeEasingSlotName)
  , mBeatQuantizerFrequency(this, BeatQuantizerFrequencySlotName)
  , mSceneTimeNode(make_shared<SceneTimeNode>())
{
  mTimeSlot.Connect(mSceneTimeNode);
  mNoiseVelocity.SetDefaultValue(20.0f);
  mBeatSpikeEasing.SetDefaultValue(1.0f);
  mBeatSpikeLength.SetDefaultValue(0.5f);
}


//float FloatSplineNode::EvaluateLinearSpline(vector<SplinePoint>& points, float time) {
//  if (points.size() == 0) return 0.0f;
//  if (points.size() == 1) return points[0].mValue;
//
//  /// Binary search
//  UINT i1 = 0;
//  UINT i2 = UINT(points.size() - 1);
//  while (i1 < i2) {
//    UINT center = (i1 + i2 + 1) / 2;
//    if (points[center].mTime > time) {
//      i2 = center - 1;
//    } else {
//      i1 = center;
//    }
//  }
//
//  SplinePoint& p1 = points[i1];
//  SplinePoint& p2 = points[i1 + 1];
//  if (p2.mTime - p1.mTime < Epsilon) return p1.mValue;
//  return p1.mValue + (p2.mValue - p1.mValue) * (time - p1.mTime) / (p2.mTime - p1.mTime);
//}

template <>
float SplineNode<ValueType::FLOAT>::GetValue(float time) {
  return mBaseLayer.Get(time) + GetNoiseValue(time) + GetBeatSpikeValue(time) +
    GetBeatQuantizerValue(time);
}

template <>
Vec3 SplineNode<ValueType::VEC3>::GetValue(float time) {
  return mBaseLayer.Get(time) + Vec3::From(GetNoiseValue(time)) + 
    Vec3::From(GetBeatSpikeValue(time)) + Vec3::From(GetBeatQuantizerValue(time));
}
