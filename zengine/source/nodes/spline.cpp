#include <include/nodes/spline.h>

const float Epsilon = 0.0001f;


int AbstractSpline::FindPointIndexBefore(float time) {
  int i = mLastIndex;
  const int count = GetControlPointCount();
  if (i >= count) i = -1;
  while (i < count - 1 && GetPointTime(i + 1) <= time) i++;
  while (i >= 0 && GetPointTime(i) > time) i--;
  mLastIndex = i;
  return i;
}

template <typename Vector>
Vector VectorSpline<Vector>::Evaluate(float time)
{
  int index = FindPointIndexBefore(time);

  Point* before = index >= 0 ? &mPoints[index] : nullptr;
  Point* after = index < int(mPoints.size()) - 1 ? &mPoints[index + 1] : nullptr;

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

      Vector ea = before->mValue;
      Vector eb = before->mTangentAfter * dt;
      Vector ec = (after->mValue - before->mValue) * 3.0f -
        (before->mTangentAfter * 2.0f + after->mTangentBefore) * dt;
      Vector ed = (after->mValue - before->mValue) * -2.0f +
        (before->mTangentAfter + after->mTangentBefore) * dt;

      return ea + eb * ft + ec * ft * ft + ed * ft * ft * ft;
    }
  return before ? before->mValue : after ? after->mValue : Vector();
}

template <typename Vector>
int VectorSpline<Vector>::GetControlPointCount() {
  return int(mPoints.size());
}

template <>
int VectorSpline<float>::GetDimensionCount() {
  return 1;
}

template <typename Vector>
int VectorSpline<Vector>::GetDimensionCount() {
  return Vector::Dimensions;
}

template <>
void VectorSpline<float>::SetPointTangentAfter(int index, int dimension, Vec2 tangent) {
  mPoints[index].mTangentAfter = tangent.y / tangent.x;
}

template <typename Vector>
void VectorSpline<Vector>::SetPointTangentAfter(int index, int dimension, Vec2 tangent) {
  mPoints[index].mTangentAfter[dimension] = tangent.y / tangent.x;
}

template <>
void VectorSpline<float>::SetPointTangentBefore(int index, int dimension, Vec2 tangent) {
  mPoints[index].mTangentBefore = tangent.y / tangent.x;
}

template <typename Vector>
void VectorSpline<Vector>::SetPointTangentBefore(int index, int dimension, Vec2 tangent) {
  mPoints[index].mTangentBefore[dimension] = tangent.y / tangent.x;
}

template <>
Vec2 VectorSpline<float>::GetPointTangentBefore(int index, int dimension) {
  return Vec2(-1, mPoints[index].mTangentBefore);
}

template <typename Vector>
Vec2 VectorSpline<Vector>::GetPointTangentBefore(int index, int dimension) {
  return Vec2(-1, mPoints[index].mTangentBefore[dimension]);
}

template <>
Vec2 VectorSpline<float>::GetPointTangentAfter(int index, int dimension) {
  return Vec2(-1, mPoints[index].mTangentAfter);
}

template <typename Vector>
Vec2 VectorSpline<Vector>::GetPointTangentAfter(int index, int dimension) {
  return Vec2(-1, mPoints[index].mTangentAfter[dimension]);
}

template <>
void VectorSpline<float>::SetPointValue(int index, int dimension, float value) {
  mPoints[index].mValue = value;
}

template <typename Vector>
void VectorSpline<Vector>::SetPointValue(int index, int dimension, float value) {
  mPoints[index].mValue[dimension] = value;
}

template <>
float VectorSpline<float>::GetPointValue(int index, int dimension) {
  return mPoints[index].mValue;
}

template <typename Vector>
float VectorSpline<Vector>::GetPointValue(int index, int dimension) {
  return mPoints[index].mValue[dimension];
}

template <typename Vector>
float VectorSpline<Vector>::GetPointTime(int index) {
  return mPoints[index].mTime;
}

template <>
float VectorSpline<float>::EvaluateDimension(float time, int dimension) {
  return Evaluate(time);
}

template <typename Vector>
float VectorSpline<Vector>::EvaluateDimension(float time, int dimension) {
  return Evaluate(time)[dimension];
}

template class VectorSpline<float>;
template class VectorSpline<Vec2>;
template class VectorSpline<Vec3>;
template class VectorSpline<Vec4>;
