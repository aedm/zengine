// general/SSpline.cpp

#include <include/nodes/splinenode.h>
#include <include/nodes/scenenode.h>
#include <math.h>

REGISTER_NODECLASS(FloatSplineNode, "Float Spline");

const float Epsilon = 0.0001f;

static SharedString TimeSlotName = make_shared<string>("Time");
static SharedString PropertiesSlotName = make_shared<string>("Properties");
static SharedString NoiseEnabledSlotName = make_shared<string>("Noise enabled");
static SharedString NoiseVelocitySlotName = make_shared<string>("Noise velocity");
static SharedString BeatSpikeEnabledSlotName = make_shared<string>("Beat spike enabled");
static SharedString BeatSpikeLengthSlotName = make_shared<string>("Beat spike length");
static SharedString BeatSpikeEasingSlotName = make_shared<string>("Beat spike easing");
static SharedString BeatQuantizerFrequencySlotName = make_shared<string>("Quantizer freq");

SplinePoint::SplinePoint() {
  mTangentBefore = mTangentAfter = 0.0f;
  mTime = 0.0f;
  mValue = 0.0f;
}


void SplinePoint::SetValue(float time, float value) {
  this->mTime = time;
  this->mValue = value;
}


FloatSplineNode::FloatSplineNode()
  : ValueNode<float>()
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


FloatSplineNode::~FloatSplineNode() {
  mSceneTimeNode->Dispose();
}


void FloatSplineNode::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::VALUE_CHANGED:
      InvalidateCurrentValue();
      NotifyWatchers(&Watcher::OnTimeEdited, mSceneTimeNode->Get());
      break;
    default:
      break;
  }
}

const float& FloatSplineNode::Get() {
  Update();
  return currentValue;
}

float FloatSplineNode::GetNoiseValue(float time) {
  if (mNoiseEnabled.Get() < 0.5f) return 0.0f;
  float noiseVelocity = mNoiseVelocity.Get();
  float noiseRatio = mNoiseLayer.Get(time) * 0.33f;
  float t = time * noiseVelocity;
  return noiseRatio * (sinf(t * 0.67f) + cosf(t * 2.43f) + cosf(t * 3.81f + 0.5f));
}

float FloatSplineNode::GetBeatSpikeValue(float time) {
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
}

float FloatSplineNode::GetBeatQuantizerValue(float time) {
  float freq = mBeatQuantizerFrequency.Get();
  if (freq < Epsilon) return 0.0f;
  
  float t = freq * floorf(time / freq);
  return mBeatQuantizerLayer.Get(t);
}

int SplineFloatComponent::AddPoint(float time, float value) {
  int i = 0;
  while (i < mPoints.size() && mPoints[i].mTime <= time) i++;

  SplinePoint tmp;
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
}

void SplineFloatComponent::SetPointValue(int index, float time, float value) {
  if (index < 0 || index >= int(mPoints.size())) return;
  SplinePoint& point = mPoints[index];

  if (index > 0 && mPoints[index - 1].mTime > time) time = mPoints[index - 1].mTime;
  if (index < mPoints.size() - 1 && mPoints[index + 1].mTime < time) {
    time = mPoints[index + 1].mTime;
  }

  point.mTime = time;
  point.mValue = value;
  CalculateTangent(index - 1);
  CalculateTangent(index);
  CalculateTangent(index + 1);
}

void FloatSplineNode::SetAutotangent(SplineLayer layer, int index, bool autotangent) {
  SplineFloatComponent* component = GetComponent(layer);
  auto& points = component->mPoints;
  if (index >= 0 && index < int(points.size())) {
    SplinePoint& point = points[index];
    point.mIsAutoangent = autotangent;
    component->CalculateTangent(index);
    InvalidateCurrentValue();
  }
}

void SplineFloatComponent::CalculateTangent(int index) {
  if (index < 0 || index >= int(mPoints.size())) return;
  SplinePoint& point = mPoints[index];
  if (point.mIsAutoangent) {
    float ym = 0.0f;
    if (index > 0 && index < mPoints.size() - 1) {
      SplinePoint& prev = mPoints[index - 1];
      SplinePoint& next = mPoints[index + 1];
      float xm = next.mTime - prev.mTime;
      ym = next.mValue - prev.mValue;
      if (xm > Epsilon) ym /= xm;
    }
    point.mTangentAfter = point.mTangentBefore = ym;
  }
}

float FloatSplineNode::EvaluateLinearSpline(vector<SplinePoint>& points, float time) {
  if (points.size() == 0) return 0.0f;
  if (points.size() == 1) return points[0].mValue;

  /// Binary search
  UINT i1 = 0;
  UINT i2 = UINT(points.size() - 1);
  while (i1 < i2) {
    UINT center = (i1 + i2 + 1) / 2;
    if (points[center].mTime > time) {
      i2 = center - 1;
    } else {
      i1 = center;
    }
  }

  SplinePoint& p1 = points[i1];
  SplinePoint& p2 = points[i1 + 1];
  if (p2.mTime - p1.mTime < Epsilon) return p1.mValue;
  return p1.mValue + (p2.mValue - p1.mValue) * (time - p1.mTime) / (p2.mTime - p1.mTime);
}

void FloatSplineNode::InvalidateCurrentValue() {
  if (mIsUpToDate) {
    mIsUpToDate = false;
    SendMsg(MessageType::VALUE_CHANGED);
  }
  NotifyWatchers(&Watcher::OnRedraw);
}

void FloatSplineNode::Operate() {
  currentValue = GetValue(mTimeSlot.Get());
}

float FloatSplineNode::GetValue(float time) {
  return mBaseLayer.Get(time) + GetNoiseValue(time) + GetBeatSpikeValue(time) +
    GetBeatQuantizerValue(time);
}


const std::vector<SplinePoint>& SplineComponent::GetPoints() {
  return mPoints;
}

int SplineComponent::FindPointIndexBefore(float time) {
  int i = mLastIndex;
  if (i >= int(mPoints.size())) i = -1;
  while (i < int(mPoints.size()) - 1 && mPoints[i + 1].mTime <= time) i++;
  while (i >= 0 && mPoints[i].mTime > time) i--;
  mLastIndex = i;
  return i;
}

float SplineFloatComponent::Get(float time) {
  int index = FindPointIndexBefore(time);

  SplinePoint* before = index >= 0 ? &mPoints[index] : nullptr;
  SplinePoint* after = index < int(mPoints.size()) - 1 ? &mPoints[index + 1] : nullptr;

  if (before && after)
    if (before->mIsLinear) {
      float dt = after->mTime - before->mTime;
      if (dt <= Epsilon) return before->mValue;
      return (after->mValue * (time - before->mTime) + 
              before->mValue * (after->mTime - time)) / dt;
    } else {
      float dt = after->mTime - before->mTime;
      float ft = (time - before->mTime) / dt;

      float ea = before->mValue;
      float eb = dt * before->mTangentAfter;
      float ec = 3.0f * (after->mValue - before->mValue) - 
        dt * (2.0f * before->mTangentAfter + after->mTangentBefore);
      float ed = -2.0f * (after->mValue - before->mValue) + 
        dt * (before->mTangentAfter + after->mTangentBefore);

      return ea + ft*eb + ft*ft*ec + ft*ft*ft*ed;
    }
  return before ? before->mValue : after ? after->mValue : 0.0f;
}


SplineFloatComponent* FloatSplineNode::GetComponent(SplineLayer layer) {
  switch (layer) {
    case SplineLayer::BASE:
      return &mBaseLayer;
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
}

int FloatSplineNode::AddPoint(SplineLayer layer, float time, float value) {
  int index = GetComponent(layer)->AddPoint(time, value);
  InvalidateCurrentValue();
  return index;
}

void FloatSplineNode::SetPointValue(SplineLayer layer, int index, float time, float value) {
  GetComponent(layer)->SetPointValue(index, time, value);
  InvalidateCurrentValue();
}

void FloatSplineNode::RemovePoint(SplineLayer layer, int index) {
  auto& points = GetComponent(layer)->mPoints;
  points.erase(points.begin() + index);
  InvalidateCurrentValue();
}

void FloatSplineNode::SetBreakpoint(SplineLayer layer, int index, bool breakpoint) {
  SplineComponent* component = GetComponent(layer);
  auto& points = component->mPoints;
  if (index >= 0 && index < int(points.size())) {
    SplinePoint& point = points[index];
    point.mIsBreakpoint = breakpoint;
    component->CalculateTangent(index);
    InvalidateCurrentValue();
  }
}

void FloatSplineNode::SetLinear(SplineLayer layer, int index, bool linear) {
  SplineComponent* component = GetComponent(layer);
  auto& points = component->mPoints;
  if (index >= 0 && index < int(points.size())) {
    SplinePoint& point = points[index];
    point.mIsLinear = linear;
    InvalidateCurrentValue();
  }
}
