#include "splinewatcher.h"
#include <cmath>
#include "../zengarden.h"

static const float DefaultPixelsPerBeat = 30.0f;
static const float DefaultPixelsPerValue = 30.0f;

FloatSplineWatcher::FloatSplineWatcher(const std::shared_ptr<Node>& node)
  : WatcherUi(node) {}

void FloatSplineWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);

  mUi.setupUi(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(mUi.splineFrame);
  layout->setContentsMargins(0, 0, 0, 0);

  mSplineWidget = new EventForwarderWidget(mUi.splineFrame);
  layout->addWidget(mSplineWidget);
  mSplineWidget->mOnPaint += Delegate(this, &FloatSplineWatcher::DrawSpline);
  mSplineWidget->mOnMousePress += Delegate(this, &FloatSplineWatcher::HandleMouseDown);
  mSplineWidget->mOnMouseRelease += Delegate(this, &FloatSplineWatcher::HandleMouseUp);
  mSplineWidget->mOnMouseMove += Delegate(this, &FloatSplineWatcher::HandleMouseMove);
  mSplineWidget->mOnMouseWheel += Delegate(this, &FloatSplineWatcher::HandleMouseWheel);

  UpdateRangeLabels();
  SelectPoint(SplineLayer::NONE, -1);

  mUi.linearCheckBox->setFocusPolicy(Qt::NoFocus);
  mUi.autotangentCheckBox->setFocusPolicy(Qt::NoFocus);
  mUi.breakpointCheckBox->setFocusPolicy(Qt::NoFocus);

  mUi.addBasePointButton->setFocusPolicy(Qt::NoFocus);
  mUi.addNoisePointButton->setFocusPolicy(Qt::NoFocus);
  mUi.removePointButton->setFocusPolicy(Qt::NoFocus);
  mUi.addBeatSpikeIntensityPointButton->setFocusPolicy(Qt::NoFocus);
  mUi.addBeatSpikeFrequencyPointButton->setFocusPolicy(Qt::NoFocus);
  mUi.addBeatQuantizerPointButton->setFocusPolicy(Qt::NoFocus);

  QObject::connect(mUi.addBasePointButton, &QPushButton::pressed,
                         [=]() { AddPoint(SplineLayer::BASE); });
  QObject::connect(mUi.addNoisePointButton, &QPushButton::pressed,
                         [=]() { AddPoint(SplineLayer::NOISE); });
  QObject::connect(mUi.addBeatSpikeIntensityPointButton, &QPushButton::pressed,
                         [=]() { AddPoint(SplineLayer::BEAT_SPIKE_INTENSITY); });
  QObject::connect(mUi.addBeatSpikeFrequencyPointButton, &QPushButton::pressed,
                         [=]() { AddPoint(SplineLayer::BEAT_SPIKE_FREQUENCY); });
  QObject::connect(mUi.addBeatQuantizerPointButton, &QPushButton::pressed,
                         [=]() { AddPoint(SplineLayer::BEAT_QUANTIZER); });
  QObject::connect(mUi.removePointButton, &QPushButton::pressed,
                         [=]() { RemovePoint(); });
  QObject::connect(mUi.linearCheckBox, &QPushButton::pressed,
                         [=]() { ToggleLinear(); });
  QObject::connect(mUi.timeLineEdit, &QLineEdit::editingFinished,
                         [=]() { HandleTimeEdited(); });
  QObject::connect(mUi.valueLineEdit, &QLineEdit::editingFinished,
                         [=]() { HandleValueEdited(); });
}


void FloatSplineWatcher::OnRedraw() {
  mSplineWidget->update();
}


void FloatSplineWatcher::OnSplineControlPointsChanged() {
  SelectPoint(mSelectedLayer, mSelectedPointIndex);
}


void FloatSplineWatcher::AddPoint(SplineLayer layer) {
  std::shared_ptr<FloatSplineNode> spline = GetSpline();
  const float time = spline->mTimeSlot.Get();
  const float value = spline->GetComponent(layer)->Get(time);
  const int index = spline->AddPoint(layer, time, value);
  mSelectedPointIndex = -1;
  SelectPoint(layer, index);
}


void FloatSplineWatcher::RemovePoint() {
  if (mSelectedPointIndex >= 0) {
    std::shared_ptr<FloatSplineNode> spline = GetSpline();
    spline->RemovePoint(mSelectedLayer, mSelectedPointIndex);
    const int pointsCount = spline->GetComponent(mSelectedLayer)->GetPoints().size();
    if (mSelectedPointIndex >= pointsCount) {
      mSelectedPointIndex = pointsCount - 1;
      if (mSelectedPointIndex < 0) {
        mSelectedLayer = SplineLayer::NONE;
      }
    }
  }
}

void FloatSplineWatcher::ToggleLinear() const
{
  auto& points = GetSpline()->GetComponent(mSelectedLayer)->GetPoints();
  GetSpline()->SetLinear(mSelectedLayer, mSelectedPointIndex,
                         !points[mSelectedPointIndex].mIsLinear);
}


void FloatSplineWatcher::HandleValueEdited() const
{
  if (mSelectedPointIndex < 0) return;
  const QString uiString = mUi.valueLineEdit->text();
  bool ok;
  const float f = uiString.toFloat(&ok);
  if (ok) {
    std::shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, point.mTime, f);
    mUi.valueLineEdit->clearFocus();
  }
}


void FloatSplineWatcher::HandleTimeEdited() const
{
  if (mSelectedPointIndex < 0) return;
  const QString uiString = mUi.timeLineEdit->text();
  bool ok;
  const float f = uiString.toFloat(&ok);
  if (ok) {
    std::shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, f, point.mValue);
    mUi.timeLineEdit->clearFocus();
  }
}


std::shared_ptr<FloatSplineNode> FloatSplineWatcher::GetSpline() const
{
  return PointerCast<FloatSplineNode>(GetNode());
}


void FloatSplineWatcher::OnTimeEdited(float time) {
  mSplineWidget->update();
}


void FloatSplineWatcher::HandleMouseMove(QMouseEvent* event) {
  switch (mState) {
    case State::WINDOW_MOVE:
    {
      const QPoint diff = mOriginalMousePos - event->pos();
      mLeftCenterPoint =
        mOriginalPoint + dot(GetStepsPerPixel(), vec2(diff.x(), diff.y()));
      mSplineWidget->update();
      UpdateRangeLabels();
    }
    break;
    case State::POINT_MOVE:
    {
      const QPoint diff = event->pos() - mOriginalMousePos;
      const vec2 p = mOriginalPoint + dot(GetStepsPerPixel(), vec2(diff.x(), diff.y()));
      std::shared_ptr<FloatSplineNode> spline = PointerCast<FloatSplineNode>(GetNode());
      spline->SetPointValue(mHoveredLayer, mHoveredPointIndex, p.x, p.y);
      mSplineWidget->update();
      SelectPoint(mSelectedLayer, mSelectedPointIndex);
    }
    break;
    case State::TIME_MOVE:
      GetSpline()->mSceneTimeNode->EditTime(ScreenToTime(event->pos().x()));
      break;
    default:
    {
      const QPoint mouse = event->pos();
      std::shared_ptr<FloatSplineNode> spline = GetSpline();

      for (UINT layer = UINT(SplineLayer::BASE); layer < UINT(SplineLayer::COUNT);
           layer++) {
        auto& points = spline->GetComponent(SplineLayer(layer))->GetPoints();
        for (UINT i = 0; i < points.size(); i++) {
          const SplinePoint& p = points[i];
          QPointF pt = ToScreenCoord(p.mTime, p.mValue);
          if (abs(pt.x() - mouse.x()) <= 4.0f && abs(pt.y() - mouse.y()) <= 4.0f) {
            if (int(i) != mHoveredPointIndex && layer != UINT(mHoveredLayer)) {
              mHoveredPointIndex = i;
              mHoveredLayer = SplineLayer(layer);
              mSplineWidget->update();
            }
            return;
          }
        }
      }
      mHoveredPointIndex = -1;
      mHoveredLayer = SplineLayer::NONE;
      mSplineWidget->update();
    }
    break;
  }
}

void FloatSplineWatcher::HandleMouseDown(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void FloatSplineWatcher::HandleMouseRightDown(QMouseEvent* event) {
  mState = State::WINDOW_MOVE;
  mOriginalMousePos = event->pos();
  mOriginalPoint = mLeftCenterPoint;
}

void FloatSplineWatcher::HandleMouseLeftDown(QMouseEvent* event) {
  if (mHoveredPointIndex >= 0) {
    /// Select point
    mState = State::POINT_MOVE;
    SelectPoint(mHoveredLayer, mHoveredPointIndex);
    mOriginalMousePos = event->pos();

    std::shared_ptr<FloatSplineNode> spline = GetSpline();
    const SplinePoint& p =
      spline->GetComponent(mHoveredLayer)->GetPoints()[mHoveredPointIndex];
    mOriginalPoint = vec2(p.mTime, p.mValue);
  } else {
    /// Move time
    mState = FloatSplineWatcher::State::TIME_MOVE;
    GetSpline()->mSceneTimeNode->EditTime(ScreenToTime(event->pos().x()));
    SelectPoint(SplineLayer::NONE, -1);
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
  const vec2 before = ScreenToPoint(event->pos());
  const float zoomDelta = float(event->delta()) / (120.0f * 4.0f);
  if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
    mZoomLevel.x += zoomDelta;
  } else {
    mZoomLevel.y += zoomDelta;
  }
  const vec2 after = ScreenToPoint(event->pos());
  mLeftCenterPoint -= after - before;
  mSplineWidget->update();
  UpdateRangeLabels();
}


void FloatSplineWatcher::SelectPoint(SplineLayer layer, int index) {
  if (index < 0 && mSelectedPointIndex == index) return;
  mSelectedPointIndex = index;
  mSelectedLayer = layer;
  if (index >= 0) {
    mUi.removePointButton->setEnabled(true);
    mUi.linearCheckBox->setEnabled(true);
    mUi.linearCheckBox->setChecked(
      GetSpline()->GetComponent(layer)->GetPoints()[index].mIsLinear);
  } else {
    mUi.removePointButton->setEnabled(false);
    mUi.linearCheckBox->setEnabled(false);
  }
  UpdateTimeEdit();
  UpdateValueEdit();
  UpdateRangeLabels();
}


void FloatSplineWatcher::UpdateTimeEdit() const
{
  if (mSelectedPointIndex < 0) {
    mUi.timeLineEdit->setText("");
    mUi.timeLineEdit->setEnabled(false);
    return;
  }
  const SplinePoint& p =
    GetSpline()->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
  mUi.timeLineEdit->setEnabled(true);
  mUi.timeLineEdit->setText(QString::number(p.mTime, 'f'));
}


void FloatSplineWatcher::UpdateValueEdit() const
{
  if (mSelectedPointIndex < 0) {
    mUi.valueLineEdit->setText("");
    mUi.valueLineEdit->setEnabled(false);
    return;
  }
  const SplinePoint& p =
    GetSpline()->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
  mUi.valueLineEdit->setEnabled(true);
  mUi.valueLineEdit->setText(QString::number(p.mValue, 'f'));
}


void FloatSplineWatcher::UpdateRangeLabels() const
{
  const QString pointIndex =
    mSelectedPointIndex < 0 ? "-" : QString::number(mSelectedPointIndex);
  mUi.statusLabel->setText(QString("Point #%1, XView: %2, YCenter: %3, Zoom: (%4, %5)")
                           .arg(pointIndex,
                                QString::number(mLeftCenterPoint.x, 'f', 2),
                                QString::number(mLeftCenterPoint.y, 'f', 2),
                                QString::number(mZoomLevel.x, 'f', 2),
                                QString::number(mZoomLevel.y, 'f', 2)));
}


QPointF FloatSplineWatcher::ToScreenCoord(float time, float value) const
{
  const QPointF pps = GetPixelsPerStep();
  const float x = (time - mLeftCenterPoint.x) * pps.x();
  const float y = 0.5f * float(mSplineWidget->height()) -
    (mLeftCenterPoint.y - value) * pps.y();
  return QPointF(x, y);
}


float FloatSplineWatcher::ScreenToTime(int xPos) const
{
  return float(xPos) * GetStepsPerPixel().x + mLeftCenterPoint.x;
}

vec2 FloatSplineWatcher::ScreenToPoint(const QPoint& pos) const
{
  const vec2 spp = GetStepsPerPixel();
  const float x = float(pos.x()) * spp.x + mLeftCenterPoint.x;
  const float y = mLeftCenterPoint.y -
    (0.5f * float(mSplineWidget->height()) - float(pos.y())) * spp.y;
  return vec2(x, y);
}

vec2 FloatSplineWatcher::GetStepsPerPixel() const
{
  return vec2(powf(0.5f, mZoomLevel.x) / DefaultPixelsPerBeat,
              -powf(0.5f, mZoomLevel.y) / DefaultPixelsPerValue);
}

QPointF FloatSplineWatcher::GetPixelsPerStep() const
{
  return QPointF(powf(2.0f, mZoomLevel.x) * DefaultPixelsPerBeat,
                 -powf(2.0f, mZoomLevel.y) * DefaultPixelsPerValue);
}

static QPointF drawPoints[10000 * 2];

void FloatSplineWatcher::DrawSpline(QPaintEvent* ev) const {
  QPainter painter(mSplineWidget);
  painter.fillRect(mSplineWidget->rect(), QBrush(QColor(53, 53, 53)));
  painter.setRenderHint(QPainter::Antialiasing);

  const float height = float(mSplineWidget->height());
  const float width = float(mSplineWidget->width());

  /// Draw center axes
  painter.setPen(QColor(80, 80, 80));
  const QPointF origin = ToScreenCoord(0, 0);
  painter.drawLine(QPointF(origin.x(), 0), QPointF(origin.x(), height));
  painter.drawLine(QPointF(0, origin.y()), QPointF(width, origin.y()));

  /// Draw beats
  const float delta = powf(0.5f, roundf(mZoomLevel.x));
  float beat = delta * floorf(mLeftCenterPoint.x / delta);
  const float lastMarkToDraw = ScreenToTime(width);
  while (true) {
    const int ibeat = int(beat);
    if (beat == float(ibeat)) {
      painter.setPen(ibeat % 64 == 0 ? QColor(150, 150, 0)
                     : ibeat % 16 == 0 ? QColor(160, 40, 40)
                     : ibeat % 4 == 0 ? QColor(0, 0, 0)
                     : QColor(40, 40, 40));
    } else painter.setPen(QColor(40, 40, 40));
    const int x = ToScreenCoord(beat, 0).x();
    painter.drawLine(QPointF(x, 0), QPointF(x, height));
    beat += delta;
    if (beat >= lastMarkToDraw) break;
  }

  std::shared_ptr<FloatSplineNode> spline = GetSpline();

  /// Draw scene time
  painter.setPen(QColor(80, 200, 80));
  const QPointF timePoint = ToScreenCoord(spline->mTimeSlot.Get(), 0);
  painter.drawLine(QPointF(timePoint.x(), 0), QPointF(timePoint.x(), height));

  /// Draw spline
  painter.setPen(QColor(0, 192, 192));
  const UINT sampleCount = mSplineWidget->width();
  const vec2 spp = GetStepsPerPixel();
  const float ppsy = 1.0f / spp.y;
  const float heightHalf = 0.5f * float(mSplineWidget->height());
  float t = mLeftCenterPoint.x;
  for (UINT i = 0; i < sampleCount; i++) {
    const float splineVal = spline->GetValue(t);
    const float y = (splineVal - mLeftCenterPoint.y) * ppsy + heightHalf;
    drawPoints[i * 2] = QPointF(float(i), y);
    drawPoints[i * 2 + 1] = QPointF(float(i), y);
    t += spp.x;
  }
  painter.drawLines(drawPoints + 1, sampleCount - 1);

  /// Draw spline components
  painter.setPen(QColor(210, 210, 210));
  DrawSplineComponentControl(painter, SplineLayer::BASE);

  if (spline->mNoiseEnabled.Get() >= 0.5f) {
    painter.setPen(QColor(192, 192, 0));
    DrawSplineComponentControl(painter, SplineLayer::NOISE);
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
  QPainter& painter, SplineLayer layer) const
{
  std::shared_ptr<FloatSplineNode> spline = GetSpline();
  SplineFloatComponent* component = spline->GetComponent(layer);

  const UINT sampleCount = mSplineWidget->width();
  const vec2 spp = GetStepsPerPixel();
  const float ppsy = 1.0f / spp.y;
  const float heightHalf = 0.5f * float(mSplineWidget->height());
  float t = mLeftCenterPoint.x;
  for (UINT i = 0; i < sampleCount; i++) {
    const float splineVal = component->Get(t);
    const float y = (splineVal - mLeftCenterPoint.y) * ppsy + heightHalf;
    drawPoints[i * 2] = QPointF(float(i), y);
    drawPoints[i * 2 + 1] = QPointF(float(i), y);
    t += spp.x;
  }
  painter.drawLines(drawPoints + 1, sampleCount - 1);

  auto& points = component->GetPoints();

  /// Draw control points
  painter.setPen(QColor(255, 255, 255));
  for (UINT i = 0; i < points.size(); i++) {
    painter.setBrush((int(i) == mSelectedPointIndex && layer == mSelectedLayer)
                     ? QBrush(QColor(0, 255, 255))
                     : (int(i) == mHoveredPointIndex && layer == mHoveredLayer)
                     ? QBrush(QColor(255, 255, 0)) : Qt::NoBrush);
    const SplinePoint& point = points[i];
    QPointF p = ToScreenCoord(point.mTime, point.mValue);
    painter.drawRect(QRectF(p.x() - 4, p.y() - 4, 8, 8));
  }
}
