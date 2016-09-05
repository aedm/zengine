// general/SSpline.cpp

#include <include/nodes/splinenode.h>
#include <math.h>

REGISTER_NODECLASS(SSpline, "Float Spline");

#define EPSILON			0.0001f

static SharedString TimeSlotName = make_shared<string>("Time");

SSplinePoint::SSplinePoint()
{
  tangentBefore = tangentAfter = 0.0f;
  time = 0.0f;
  value = 0.0f;
}


void SSplinePoint::setValue(float time, float value) {
  this->time = time;
  this->value = value;
}


SSpline::SSpline()
  : ValueNode<NodeType::FLOAT>()
  , mTimeSlot(this, TimeSlotName)
{
  addPoint(10, 2);
  addPoint(20, -2);
  addPoint(50, 4);
  addPoint(80, -8);
}


SSpline::~SSpline() {}


const float& SSpline::Get() {
  currentValue = getValue(mTimeSlot.Get());
  return currentValue;
}

float SSpline::getValue(float time) {
  // search for the actual control point

  /*int beforeIndex = points[lastIndex);
  SSplinePoint& after = lastsel ? lastsel->next : points;
  while (after && after->t<=t)
  {
    before = after;
    after = after->next;
  }
  while (before && before->t>t)
  {
    after = before;
    before = before->prev;
  }*/

  if (points.size() == 0) return defaultValue;

  if (lastIndex >= int(points.size())) lastIndex = 0;
  if (lastIndex < -1) lastIndex = -1;
  while (lastIndex < int(points.size()) - 1 && points[lastIndex + 1].time <= time) {
    lastIndex++;
  }
  while (lastIndex >= 0 && points[lastIndex].time > time) {
    lastIndex--;
  }

  SSplinePoint* before = lastIndex >= 0 ? &points[lastIndex] : nullptr;
  SSplinePoint* after = lastIndex < int(points.size()) - 1 ? &points[lastIndex + 1] : nullptr;

  float v = before ? before->value : after ? after->value : defaultValue;
  if (before && after)
    if (before->isLinear) {
      float dt = after->time - before->time;
      if (dt <= EPSILON) return before->value; 
      return (after->value * (time - before->time) + before->value * (after->time - time)) / dt;
    } else {
      float dt = after->time - before->time;
      float ft = (time - before->time) / dt;

      float ea = before->value;
      float eb = dt * before->tangentAfter;
      float ec = 3 * (after->value - before->value) - dt * (2 * before->tangentAfter + after->tangentBefore);
      float ed = -2 * (after->value - before->value) + dt * (before->tangentAfter + after->tangentBefore);

      return ea + ft*eb + ft*ft*ec + ft*ft*ft*ed;
    }

  return v;
}


int SSpline::addPoint(float time, float value) {
  int i = 0;
  while (i < points.size() && points[i].time <= time) i++;

  SSplinePoint tmp;
  tmp.time = time;
  tmp.value = value;
  tmp.isAutoangent = true;
  tmp.isBreakpoint = false;
  tmp.isLinear = i > 0 ? points[i - 1].isLinear : false;

  points.insert(points.begin() + i, tmp);

  calculateTangent(i - 1);
  calculateTangent(i);
  calculateTangent(i + 1);

  return i;
}

void SSpline::setPointValue(int index, float time, float value) {
  if (index < 0 || index >= int(points.size())) return;
  SSplinePoint& point = points[index];

  if (index > 0 && points[index - 1].time > time) time = points[index - 1].time;
  if (index < points.size() - 1 && points[index + 1].time < time) {
    time = points[index + 1].time;
  }

  point.time = time;
  point.value = value;
  calculateTangent(index - 1);
  calculateTangent(index);
  calculateTangent(index + 1);
}

void SSpline::setAutotangent(int index, bool autotangent) {
  if (index >= 0 && index < int(points.size())) {
    SSplinePoint& point = points[index];
    point.isAutoangent = autotangent;
    calculateTangent(index);
  }
}

void SSpline::setBreakpoint(int index, bool breakpoint) {
  if (index >= 0 && index < int(points.size())) {
    SSplinePoint& point = points[index];
    point.isBreakpoint = breakpoint;
    calculateTangent(index);
  }
}


void SSpline::calculateTangent(int index) {
  if (index < 0 || index >= int(points.size())) return;
  SSplinePoint& point = points[index];
  if (point.isAutoangent) {
    float ym = 0.0f;
    if (index > 0 && index < points.size() - 1) {
      SSplinePoint& prev = points[index - 1];
      SSplinePoint& next = points[index + 1];
      float xm = next.time - prev.time;
      ym = next.value - prev.value;
      if (xm > EPSILON) ym /= xm;
    }
    point.tangentAfter = point.tangentBefore = ym;
  }
}

void SSpline::removePoint(int index) {
  points.erase(points.begin() + index);
}

UINT SSpline::getNumPoints() {
  return UINT(points.size());
}

const SSplinePoint& SSpline::getPoint(int index) {
  return points[index];
}
