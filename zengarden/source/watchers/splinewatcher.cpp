#include "splinewatcher.h"
#include <math.h>
#include "../zengarden.h"

static const float DefaultPixelsPerBeat = 30.0f;
static const float DefaultPixelsPerValue = 30.0f;

/// 10k spline points to draw, should be enough up to 8k displays
static const int MaxDisplayPointCount = 10000;

template<typename T>
float GetVectorComponent(T vec, int index);

template<>
float GetVectorComponent<float>(float f, int index) {
  ASSERT(index == 0);
  return f;
}

template<typename T>
float GetVectorComponent(T vec, int index) {
  return vec[index];
}

#define CALL_TEMPLATE(node, function, ...) \
(\
  IsPointerOf<SplineNode<ValueType::FLOAT>>(node) \
  ? function<ValueType::FLOAT, 1>(PointerCast<SplineNode<ValueType::FLOAT>>(node), __VA_ARGS__) \
  : IsPointerOf<SplineNode<ValueType::VEC2>>(node) \
  ? function<ValueType::VEC2, 2>(PointerCast<SplineNode<ValueType::VEC2>>(node), __VA_ARGS__) \
  : IsPointerOf<SplineNode<ValueType::VEC3>>(node) \
  ? function<ValueType::VEC3, 3>(PointerCast<SplineNode<ValueType::VEC3>>(node), __VA_ARGS__) \
  : function<ValueType::VEC4, 4>(PointerCast<SplineNode<ValueType::VEC4>>(node), __VA_ARGS__) \
)\


FloatSplineWatcher::FloatSplineWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node)
  , mDrawPoints(MaxDisplayPointCount)
  , mSplinePoints(MaxDisplayPointCount)
{}


void FloatSplineWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);

  mUI.setupUi(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(mUI.splineFrame);
  layout->setContentsMargins(0, 0, 0, 0);

  mSplineWidget = new EventForwarderWidget(mUI.splineFrame);
  layout->addWidget(mSplineWidget);
  mSplineWidget->mOnPaint += Delegate(this, &FloatSplineWatcher::DrawSpline);
  mSplineWidget->OnMousePress += Delegate(this, &FloatSplineWatcher::HandleMouseDown);
  mSplineWidget->OnMouseRelease += Delegate(this, &FloatSplineWatcher::HandleMouseUp);
  mSplineWidget->OnMouseMove += Delegate(this, &FloatSplineWatcher::HandleMouseMove);
  mSplineWidget->OnMouseWheel += Delegate(this, &FloatSplineWatcher::HandleMouseWheel);

  UpdateRangeLabels();
  SelectPoint(PointSelection::None());

  mUI.addBasePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addNoisePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.removePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.linearCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.autotangentCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.breakpointCheckBox->setFocusPolicy(Qt::NoFocus);

  watcherWidget->connect(mUI.addBasePointButton, &QPushButton::pressed,
    [=]() { AddPoint(SplineLayer::BASE); });
  watcherWidget->connect(mUI.addNoisePointButton, &QPushButton::pressed,
    [=]() { AddPoint(SplineLayer::NOISE); });
  watcherWidget->connect(mUI.addBeatSpikeIntensityPointButton, &QPushButton::pressed,
    [=]() { AddPoint(SplineLayer::BEAT_SPIKE_INTENSITY); });
  watcherWidget->connect(mUI.addBeatSpikeFrequencyPointButton, &QPushButton::pressed,
    [=]() { AddPoint(SplineLayer::BEAT_SPIKE_FREQUENCY); });
  watcherWidget->connect(mUI.addBeatQuantizerPointButton, &QPushButton::pressed,
    [=]() { AddPoint(SplineLayer::BEAT_QUANTIZER); });
  watcherWidget->connect(mUI.removePointButton, &QPushButton::pressed,
    [=]() { RemovePoint(); });
  watcherWidget->connect(mUI.linearCheckBox, &QPushButton::pressed,
    [=]() { ToggleLinear(); });
  watcherWidget->connect(mUI.timeLineEdit, &QLineEdit::editingFinished,
    [=]() { HandleTimeEdited(); });
  watcherWidget->connect(mUI.valueLineEdit, &QLineEdit::editingFinished,
    [=]() { HandleValueEdited(); });
}


void FloatSplineWatcher::OnRedraw() {
  mSplineWidget->update();
}


void FloatSplineWatcher::OnSplineControlPointsChanged() {
  SelectPoint(PointSelection::None());
}

void FloatSplineWatcher::AddPoint(SplineLayer layer) {
  CALL_TEMPLATE(GetNode(), AddPoint, layer);
}

template<ValueType V, int ComponentCount>
void FloatSplineWatcher::AddPoint(
  shared_ptr<SplineNode<V>> spline, SplineLayer layer) 
{
  float time = spline->mTimeSlot.Get();
  int index = -1;
  if (layer == SplineLayer::BASE) {
    ValueTypes<V>::Type value = spline->mBaseLayer.Get(time);
    index = spline->AddBasePoint(time, value);
  }
  else {
    float value = spline->GetComponent(layer)->Get(time);
    index = spline->AddLayerPoint(layer, time, value);
  }
  SelectPoint(PointSelection(layer, index, 0));
}


void FloatSplineWatcher::RemovePoint() {
  if (!mSelectedPoint.isValid()) return;
  shared_ptr<FloatSplineNode> spline = GetSpline();
  spline->RemovePoint(mSelectedPoint.mLayer, mSelectedPoint.mIndex);
  int pointsCount = spline->GetComponent(mSelectedPoint.mLayer)->GetPoints().size();
  if (mSelectedPoint.mIndex >= pointsCount) {
    if (pointsCount == 0) {
      SelectPoint(PointSelection::None());
    }
    else {
      PointSelection selection = mSelectedPoint;
      selection.mIndex--;
      SelectPoint(selection);
    }
  }
}

bool FloatSplineWatcher::IsSplinePointLinear(SplineLayer layer, int index) {
  return CALL_TEMPLATE(GetNode(), IsSplinePointLinear, layer, index);
}

template<ValueType V, int ComponentCount>
bool FloatSplineWatcher::IsSplinePointLinear(
  shared_ptr<SplineNode<V>> splineNode, SplineLayer layer, int index)
{
  if (layer == SplineLayer::BASE) {
    return splineNode->mBaseLayer.GetPoints()[index].mIsLinear;
  }
  return splineNode->GetComponent(layer)->GetPoints()[index].mIsLinear;
}


void FloatSplineWatcher::TogglePointLinear(SplineLayer layer, int index) {
  CALL_TEMPLATE(GetNode(), TogglePointLinear, layer, index);
}

template<ValueType V, int ComponentCount>
void FloatSplineWatcher::TogglePointLinear(
  shared_ptr<SplineNode<V>> splineNode, SplineLayer layer, int index)
{
  bool linear = (layer == SplineLayer::BASE) 
    ? splineNode->mBaseLayer.GetPoints()[index].mIsLinear
    : splineNode->GetComponent(layer)->GetPoints()[index].mIsLinear;
  splineNode->SetLinear(layer, index, !linear);
}


void FloatSplineWatcher::ToggleLinear() {
  TogglePointLinear(mSelectedPoint.mLayer, mSelectedPoint.mIndex);
}


template<ValueType V, int ComponentCount>
float FloatSplineWatcher::GetPointTime(
  shared_ptr<SplineNode<V>> splineNode, SplineLayer layer, int index)
{
  if (layer == SplineLayer::BASE) {
    return splineNode->mBaseLayer.GetPoints()[index].mTime;
  }
  return splineNode->GetComponent(layer)->GetPoints()[index].mTime;
}

float FloatSplineWatcher::GetPointTime(SplineLayer layer, int index) {
  return CALL_TEMPLATE(GetNode(), GetPointTime, layer, index);
}


template<ValueType V, int ComponentCount>
float FloatSplineWatcher::GetPointValue(
  shared_ptr<SplineNode<V>> splineNode, PointSelection& selection)
{
  if (selection.mLayer == SplineLayer::BASE) {
    return GetVectorComponent(
      splineNode->mBaseLayer.GetPoints()[selection.mIndex].mValue, selection.mComponent);
  }
  return splineNode->GetComponent(selection.mLayer)->
    GetPoints()[selection.mIndex].mValue;
}

float FloatSplineWatcher::GetPointValue(PointSelection& selection) {
  return CALL_TEMPLATE(GetNode(), GetPointValue, selection);
}


void FloatSplineWatcher::HandleValueEdited() {
  if (!mSelectedPoint.isValid()) return;
  QString uiString = mUI.valueLineEdit->text();
  bool ok;
  float f = uiString.toFloat(&ok);
  if (ok) {
    shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = 
      spline->GetComponent(mSelectedPoint.mLayer)->GetPoints()[mSelectedPoint.mIndex];
    if (mSelectedPoint.mLayer == SplineLayer::BASE) {
      spline->SetBaseValue(mSelectedPoint.mIndex, point.mTime, f);
    }
    else {
      spline->SetLayerValue(
        mSelectedPoint.mLayer, mSelectedPoint.mIndex, point.mTime, f);
    }
    mUI.valueLineEdit->clearFocus();
  }
}


void FloatSplineWatcher::HandleTimeEdited() {
  if (!mSelectedPoint.isValid()) return;
  QString uiString = mUI.timeLineEdit->text();
  bool ok;
  float f = uiString.toFloat(&ok);
  if (ok) {
    shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = 
      spline->GetComponent(mSelectedPoint.mLayer)->GetPoints()[mSelectedPoint.mIndex];
    if (mSelectedPoint.mLayer == SplineLayer::BASE) {
      spline->SetBaseValue(mSelectedPoint.mIndex, f, point.mValue);
    }
    else {
      spline->SetLayerValue(
        mSelectedPoint.mLayer, mSelectedPoint.mIndex, f, point.mValue);
    }
    mUI.timeLineEdit->clearFocus();
  }
}


shared_ptr<FloatSplineNode> FloatSplineWatcher::GetSpline() {
  return PointerCast<FloatSplineNode>(GetNode());
}


void FloatSplineWatcher::OnTimeEdited(float time) {
  mSplineWidget->update();
}


void FloatSplineWatcher::HandleMouseMove(QMouseEvent* event) {
  QSize size = mSplineWidget->size();
  switch (mState) {
  case State::WINDOW_MOVE:
  {
    QPoint diff = mOriginalMousePos - event->pos();
    mLeftCenterPoint =
      mOriginalPoint + GetStepsPerPixel().Dot(Vec2(diff.x(), diff.y()));
    mSplineWidget->update();
    UpdateRangeLabels();
  }
  break;
  case State::POINT_MOVE:
  {
    float height = float(mSplineWidget->height());
    float width = float(mSplineWidget->width());
    QPoint diff = event->pos() - mOriginalMousePos;
    Vec2 p = mOriginalPoint + GetStepsPerPixel().Dot(Vec2(diff.x(), diff.y()));
    shared_ptr<FloatSplineNode> spline = PointerCast<FloatSplineNode>(GetNode());
    if (mSelectedPoint.mLayer == SplineLayer::BASE) {
      spline->SetBaseValue(mSelectedPoint.mIndex, p.x, p.y);
    }
    else {
      spline->SetLayerValue(mSelectedPoint.mLayer, mSelectedPoint.mIndex, p.x, p.y);
    }
    mSplineWidget->update();
    SelectPoint(mSelectedPoint, true);
  }
  break;
  case State::TIME_MOVE:
    GetSpline()->mSceneTimeNode->EditTime(ScreenToTime(event->pos().x()));
    break;
  default:
  {
    QPoint mouse = event->pos();
    shared_ptr<FloatSplineNode> spline = GetSpline();
    FindHover<ValueType::FLOAT, 1>(spline, mouse);
  }
  break;
  }
}


template<typename T>
bool FloatSplineWatcher::CheckMouseOverLayer(
  Spline<T>* component, SplineLayer layer, QPoint mouse, int componentCount)
{
  auto& points = component->GetPoints();
  for (int i = 0; i < points.size(); i++) {
    const SplinePoint<T>& p = points[i];
    for (int c = 0; c < componentCount; c++) {
      QPointF pt = ToScreenCoord(p.mTime, GetVectorComponent<T>(p.mValue, c));
      if (abs(pt.x() - mouse.x()) <= 4.0f && abs(pt.y() - mouse.y()) <= 4.0f) {
        PointSelection point(layer, i, c);
        if (!(mHoveredPoint == point)) {
          mHoveredPoint = point;
          return true;
        }
        return false;
      }
    }
  }
  return false;
}

template<ValueType T, int ComponentCount>
void FloatSplineWatcher::FindHover(shared_ptr<SplineNode<T>> splineNode, QPoint mouse)
{
  mHoveredPoint = PointSelection::None();
  if (!CheckMouseOverLayer(&splineNode->mBaseLayer, SplineLayer::BASE, mouse, 
    ComponentCount)) 
  {
    for (UINT layer = UINT(SplineLayer::BASE) + 1; layer < UINT(SplineLayer::COUNT);
      layer++)
    {
      if (CheckMouseOverLayer(
        splineNode->GetComponent(SplineLayer(layer)), SplineLayer(layer), mouse, 1)) {
        mSplineWidget->update();
        break;
      }
    }
  }
  mSplineWidget->update();
}


template<ValueType T, int ComponentCount>
bool CheckMouseOverPoint(shared_ptr<SplineNode<T>> splineNode, QPoint mouse);


void FloatSplineWatcher::HandleMouseDown(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  }
  else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void FloatSplineWatcher::HandleMouseRightDown(QMouseEvent* event) {
  mState = State::WINDOW_MOVE;
  mOriginalMousePos = event->pos();
  mOriginalPoint = mLeftCenterPoint;
}

void FloatSplineWatcher::HandleMouseLeftDown(QMouseEvent* event) {
  if (mHoveredPoint.isValid()) {
    /// Select point
    mState = State::POINT_MOVE;
    SelectPoint(mHoveredPoint);
    mOriginalMousePos = event->pos();

    shared_ptr<FloatSplineNode> spline = GetSpline();
    const SplinePoint<float>& p = (mSelectedPoint.mLayer == SplineLayer::BASE) ?
      spline->mBaseLayer.GetPoints()[mSelectedPoint.mIndex] :
      spline->GetComponent(mSelectedPoint.mLayer)->GetPoints()[mSelectedPoint.mIndex];
    mOriginalPoint = Vec2(p.mTime, p.mValue);
  }
  else {
    /// Move time
    mState = FloatSplineWatcher::State::TIME_MOVE;
    GetSpline()->mSceneTimeNode->EditTime(ScreenToTime(event->pos().x()));
    SelectPoint(PointSelection::None());
  }
}


void FloatSplineWatcher::HandleMouseUp(QMouseEvent* event) {
  switch (mState) {
  case State::WINDOW_MOVE:
    break;
  default: break;
  }
  mState = State::DEFAULT;
}


void FloatSplineWatcher::HandleMouseWheel(QWheelEvent* event) {
  Vec2 before = ScreenToPoint(event->pos());
  float zoomDelta = float(event->delta()) / (120.0f * 4.0f);
  if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
    mZoomLevel.x += zoomDelta;
  }
  else {
    mZoomLevel.y += zoomDelta;
  }
  Vec2 after = ScreenToPoint(event->pos());
  mLeftCenterPoint -= after - before;
  mSplineWidget->update();
  UpdateRangeLabels();
}


void FloatSplineWatcher::SelectPoint(const PointSelection& selection, bool force) {
  if (!force && mSelectedPoint == selection) return;
  mSelectedPoint = selection;
  if (selection.mIndex >= 0) {
    mUI.removePointButton->setEnabled(true);
    mUI.linearCheckBox->setEnabled(true);
    mUI.linearCheckBox->setChecked(
      IsSplinePointLinear(mSelectedPoint.mLayer, mSelectedPoint.mIndex));
  }
  else {
    mUI.removePointButton->setEnabled(false);
    mUI.linearCheckBox->setEnabled(false);
  }
  UpdateTimeEdit();
  UpdateValueEdit();
  UpdateRangeLabels();
}


void FloatSplineWatcher::UpdateTimeEdit() {
  if (!mSelectedPoint.isValid()) {
    mUI.timeLineEdit->setText("");
    mUI.timeLineEdit->setEnabled(false);
    return;
  }
  float time = GetPointTime(mSelectedPoint.mLayer, mSelectedPoint.mIndex);
  mUI.timeLineEdit->setEnabled(true);
  mUI.timeLineEdit->setText(QString::number(time, 'f'));
}


void FloatSplineWatcher::UpdateValueEdit() {
  if (!mSelectedPoint.isValid()) {
    mUI.valueLineEdit->setText("");
    mUI.valueLineEdit->setEnabled(false);
    return;
  }
  float value = GetPointValue(mSelectedPoint);
  mUI.valueLineEdit->setEnabled(true);
  mUI.valueLineEdit->setText(QString::number(value, 'f'));
}


void FloatSplineWatcher::UpdateRangeLabels() {
  QString pointIndex =
    mSelectedPoint.isValid() ? QString::number(mSelectedPoint.mIndex) : "-";
  mUI.statusLabel->setText(QString("Point #%1, XView: %2, YCenter: %3, Zoom: (%4, %5)")
    .arg(pointIndex,
      QString::number(mLeftCenterPoint.x, 'f', 2),
      QString::number(mLeftCenterPoint.y, 'f', 2),
      QString::number(mZoomLevel.x, 'f', 2),
      QString::number(mZoomLevel.y, 'f', 2)));
}


QPointF FloatSplineWatcher::ToScreenCoord(float time, float value) {
  QPointF pps = GetPixelsPerStep();
  float x = (time - mLeftCenterPoint.x) * pps.x();
  float y = 0.5f * float(mSplineWidget->height()) -
    (mLeftCenterPoint.y - value) * pps.y();
  return QPointF(x, y);
}


float FloatSplineWatcher::ScreenToTime(int xPos) {
  return float(xPos) * GetStepsPerPixel().x + mLeftCenterPoint.x;
}

Vec2 FloatSplineWatcher::ScreenToPoint(QPoint& pos) {
  Vec2 spp = GetStepsPerPixel();
  float x = float(pos.x()) * spp.x + mLeftCenterPoint.x;
  float y = mLeftCenterPoint.y -
    (0.5f * float(mSplineWidget->height()) - float(pos.y())) * spp.y;
  return Vec2(x, y);
}

Vec2 FloatSplineWatcher::GetStepsPerPixel() {
  return Vec2(powf(0.5f, mZoomLevel.x) / DefaultPixelsPerBeat,
    -powf(0.5f, mZoomLevel.y) / DefaultPixelsPerValue);
}

QPointF FloatSplineWatcher::GetPixelsPerStep() {
  return QPointF(powf(2.0f, mZoomLevel.x) * DefaultPixelsPerBeat,
    -powf(2.0f, mZoomLevel.y) * DefaultPixelsPerValue);
}


void FloatSplineWatcher::DrawSpline(QPaintEvent* ev) {
  QPainter painter(mSplineWidget);
  painter.fillRect(mSplineWidget->rect(), QBrush(QColor(53, 53, 53)));
  painter.setRenderHint(QPainter::Antialiasing);

  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());

  /// Draw center axes
  painter.setPen(QColor(80, 80, 80));
  QPointF origo = ToScreenCoord(0, 0);
  painter.drawLine(QPointF(origo.x(), 0), QPointF(origo.x(), height));
  painter.drawLine(QPointF(0, origo.y()), QPointF(width, origo.y()));

  /// Draw beats
  float delta = powf(0.5f, roundf(mZoomLevel.x));
  float beat = delta * floorf(mLeftCenterPoint.x / delta);
  float lastMarkToDraw = ScreenToTime(width);
  while (true) {
    int ibeat = int(beat);
    if (beat == float(ibeat)) {
      painter.setPen(ibeat % 64 == 0 ? QColor(150, 150, 0)
        : ibeat % 16 == 0 ? QColor(160, 40, 40)
        : ibeat % 4 == 0 ? QColor(0, 0, 0)
        : QColor(40, 40, 40));
    }
    else painter.setPen(QColor(40, 40, 40));
    int x = ToScreenCoord(beat, 0).x();
    painter.drawLine(QPointF(x, 0), QPointF(x, height));
    beat += delta;
    if (beat >= lastMarkToDraw) break;
  }

  shared_ptr<FloatSplineNode> spline = GetSpline();

  /// Draw scene time
  painter.setPen(QColor(80, 200, 80));
  QPointF timePoint = ToScreenCoord(spline->mTimeSlot.Get(), 0);
  painter.drawLine(QPointF(timePoint.x(), 0), QPointF(timePoint.x(), height));

  /// Draw spline
  painter.setPen(QColor(0, 192, 192));
  UINT sampleCount = mSplineWidget->width();
  Vec2 spp = GetStepsPerPixel();
  float ppsy = 1.0f / spp.y;
  float heightHalf = 0.5f * float(mSplineWidget->height());
  float t = mLeftCenterPoint.x;
  for (UINT i = 0; i < sampleCount; i++) {
    float splineVal = spline->GetValue(t);
    float y = (splineVal - mLeftCenterPoint.y) * ppsy + heightHalf;
    mDrawPoints[i] = QPointF(float(i), y);
    t += spp.x;
  }
  painter.drawPolyline(&mDrawPoints[0], sampleCount);

  /// Draw spline components
  painter.setPen(QColor(210, 210, 210));
  DrawFloatSpline(painter, &spline->mBaseLayer, SplineLayer::BASE);

  if (spline->mNoiseEnabled.Get() >= 0.5f) {
    painter.setPen(QColor(192, 192, 0));
    DrawFloatSpline(painter, spline->GetComponent(SplineLayer::NOISE), SplineLayer::NOISE);
  }

  if (spline->mBeatSpikeEnabled.Get() >= 0.5f) {
    painter.setPen(QColor(192, 0, 0));
    DrawSplineComponentControl(painter, SplineLayer::BEAT_SPIKE_INTENSITY);

    painter.setPen(QColor(192, 64, 64));
    DrawSplineComponentControl(painter, SplineLayer::BEAT_SPIKE_FREQUENCY);
  }

  if (spline->mBeatQuantizerFrequency.Get() > 0.0f) {
    painter.setPen(QColor(0, 0, 255));
    DrawSplineComponentControl(painter, SplineLayer::BEAT_QUANTIZER);
  }
}


void FloatSplineWatcher::DrawSplineComponentControl(
  QPainter& painter, SplineLayer layer) 
{
  DrawFloatSpline(painter, GetSpline()->GetComponent(layer), layer);
}

//  shared_ptr<FloatSplineNode> spline = GetSpline();
//  Spline<float>* component = spline->GetComponent(layer);
//  float height = float(mSplineWidget->height());
//  float width = float(mSplineWidget->width());
//
//  UINT sampleCount = mSplineWidget->width();
//  Vec2 spp = GetStepsPerPixel();
//  float ppsy = 1.0f / spp.y;
//  float heightHalf = 0.5f * float(mSplineWidget->height());
//  float t = mLeftCenterPoint.x;
//  for (UINT i = 0; i < sampleCount; i++) {
//    float splineVal = component->Get(t);
//    float y = (splineVal - mLeftCenterPoint.y) * ppsy + heightHalf;
//    drawPoints[i * 2] = QPointF(float(i), y);
//    drawPoints[i * 2 + 1] = QPointF(float(i), y);
//    t += spp.x;
//  }
//  painter.drawLines(drawPoints + 1, sampleCount - 1);
//
//  auto& points = component->GetPoints();
//
//  /// Draw control points
//  painter.setPen(QColor(255, 255, 255));
//  for (UINT i = 0; i < points.size(); i++) {
//    painter.setBrush((i == mSelectedPointIndex && layer == mSelectedLayer)
//      ? QBrush(QColor(0, 255, 255))
//      : (i == mHoveredPointIndex && layer == mHoveredLayer)
//      ? QBrush(QColor(255, 255, 0)) : Qt::NoBrush);
//    const SplinePoint<float>& point = points[i];
//    QPointF p = ToScreenCoord(point.mTime, point.mValue);
//    painter.drawRect(QRectF(p.x() - 4, p.y() - 4, 8, 8));
//  }
//}


void FloatSplineWatcher::DrawFloatSpline(QPainter& painter, Spline<float>* component, 
  SplineLayer layer)
{
  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());

  UINT sampleCount = mSplineWidget->width();
  Vec2 spp = GetStepsPerPixel();
  float ppsy = 1.0f / spp.y;
  float heightHalf = 0.5f * float(mSplineWidget->height());
  float t = mLeftCenterPoint.x;
  for (UINT i = 0; i < sampleCount; i++) {
    float splineVal = component->Get(t);
    float y = (splineVal - mLeftCenterPoint.y) * ppsy + heightHalf;
    mDrawPoints[i] = QPointF(float(i), y);
    t += spp.x;
  }
  painter.drawPolyline(&mDrawPoints[0], sampleCount);

  auto& points = component->GetPoints();

  /// Draw control points
  painter.setPen(QColor(255, 255, 255));
  for (UINT i = 0; i < points.size(); i++) {
    PointSelection selection = PointSelection(layer, i, 0);
    painter.setBrush(mSelectedPoint == selection
      ? QBrush(QColor(0, 255, 255))
      : mHoveredPoint == selection
      ? QBrush(QColor(255, 255, 0)) : Qt::NoBrush);
    const SplinePoint<float>& point = points[i];
    QPointF p = ToScreenCoord(point.mTime, point.mValue);
    painter.drawRect(QRectF(p.x() - 4, p.y() - 4, 8, 8));
  }
}

FloatSplineWatcher::PointSelection::PointSelection(
  SplineLayer layer, int index, int component) 
  : mLayer(layer)
  , mIndex(index)
  , mComponent(component)
{}

bool FloatSplineWatcher::PointSelection::operator==(const PointSelection& op) {
  return mIndex == op.mIndex &&
    mComponent == op.mComponent &&
    mLayer == op.mLayer;
}

bool FloatSplineWatcher::PointSelection::isValid() {
  return mIndex >= 0;
}

void FloatSplineWatcher::PointSelection::inValidate() {
  mIndex = -1;
}

FloatSplineWatcher::PointSelection FloatSplineWatcher::PointSelection::None() {
  return PointSelection(SplineLayer::NONE, -1, -1);
}

