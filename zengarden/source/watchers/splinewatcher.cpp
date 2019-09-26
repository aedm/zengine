#include "splinewatcher.h"
#include <math.h>
#include "../zengarden.h"

static const float DefaultPixelsPerBeat = 30.0f;
static const float DefaultPixelsPerValue = 30.0f;

FloatSplineWatcher::FloatSplineWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node) {}

FloatSplineWatcher::~FloatSplineWatcher() {}

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
  SelectPoint(SplineLayer::NONE, -1);

  mUI.linearCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.autotangentCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.breakpointCheckBox->setFocusPolicy(Qt::NoFocus);

  mUI.addBasePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addNoisePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.removePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addBeatSpikeIntensityPointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addBeatSpikeFrequencyPointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addBeatQuantizerPointButton->setFocusPolicy(Qt::NoFocus);

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
  SelectPoint(mSelectedLayer, mSelectedPointIndex);
}


void FloatSplineWatcher::AddPoint(SplineLayer layer) {
  shared_ptr<FloatSplineNode> spline = GetSpline();
  const float time = spline->mTimeSlot.Get();
  const float value = spline->GetComponent(layer)->Get(time);
  const int index = spline->AddPoint(layer, time, value);
  mSelectedPointIndex = -1;
  SelectPoint(layer, index);
}


void FloatSplineWatcher::RemovePoint() {
  if (mSelectedPointIndex >= 0) {
    shared_ptr<FloatSplineNode> spline = GetSpline();
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
  const QString uiString = mUI.valueLineEdit->text();
  bool ok;
  const float f = uiString.toFloat(&ok);
  if (ok) {
    shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, point.mTime, f);
    mUI.valueLineEdit->clearFocus();
  }
}


void FloatSplineWatcher::HandleTimeEdited() const
{
  if (mSelectedPointIndex < 0) return;
  const QString uiString = mUI.timeLineEdit->text();
  bool ok;
  const float f = uiString.toFloat(&ok);
  if (ok) {
    shared_ptr<FloatSplineNode> spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, f, point.mValue);
    mUI.timeLineEdit->clearFocus();
  }
}


shared_ptr<FloatSplineNode> FloatSplineWatcher::GetSpline() const
{
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
      const QPoint diff = mOriginalMousePos - event->pos();
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
      const QPoint diff = event->pos() - mOriginalMousePos;
      const Vec2 p = mOriginalPoint + GetStepsPerPixel().Dot(Vec2(diff.x(), diff.y()));
      shared_ptr<FloatSplineNode> spline = PointerCast<FloatSplineNode>(GetNode());
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
      shared_ptr<FloatSplineNode> spline = GetSpline();

      for (UINT layer = UINT(SplineLayer::BASE); layer < UINT(SplineLayer::COUNT);
           layer++) {
        auto& points = spline->GetComponent(SplineLayer(layer))->GetPoints();
        for (UINT i = 0; i < points.size(); i++) {
          const SplinePoint& p = points[i];
          QPointF pt = ToScreenCoord(p.mTime, p.mValue);
          if (abs(pt.x() - mouse.x()) <= 4.0f && abs(pt.y() - mouse.y()) <= 4.0f) {
            if (i != mHoveredPointIndex && layer != UINT(mHoveredLayer)) {
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

    shared_ptr<FloatSplineNode> spline = GetSpline();
    const SplinePoint& p =
      spline->GetComponent(mHoveredLayer)->GetPoints()[mHoveredPointIndex];
    mOriginalPoint = Vec2(p.mTime, p.mValue);
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
  const Vec2 before = ScreenToPoint(event->pos());
  const float zoomDelta = float(event->delta()) / (120.0f * 4.0f);
  if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
    mZoomLevel.x += zoomDelta;
  } else {
    mZoomLevel.y += zoomDelta;
  }
  const Vec2 after = ScreenToPoint(event->pos());
  mLeftCenterPoint -= after - before;
  mSplineWidget->update();
  UpdateRangeLabels();
}


void FloatSplineWatcher::SelectPoint(SplineLayer layer, int index) {
  if (index < 0 && mSelectedPointIndex == index) return;
  mSelectedPointIndex = index;
  mSelectedLayer = layer;
  if (index >= 0) {
    mUI.removePointButton->setEnabled(true);
    mUI.linearCheckBox->setEnabled(true);
    mUI.linearCheckBox->setChecked(
      GetSpline()->GetComponent(layer)->GetPoints()[index].mIsLinear);
  } else {
    mUI.removePointButton->setEnabled(false);
    mUI.linearCheckBox->setEnabled(false);
  }
  UpdateTimeEdit();
  UpdateValueEdit();
  UpdateRangeLabels();
}


void FloatSplineWatcher::UpdateTimeEdit() const
{
  if (mSelectedPointIndex < 0) {
    mUI.timeLineEdit->setText("");
    mUI.timeLineEdit->setEnabled(false);
    return;
  }
  const SplinePoint& p =
    GetSpline()->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
  mUI.timeLineEdit->setEnabled(true);
  mUI.timeLineEdit->setText(QString::number(p.mTime, 'f'));
}


void FloatSplineWatcher::UpdateValueEdit() const
{
  if (mSelectedPointIndex < 0) {
    mUI.valueLineEdit->setText("");
    mUI.valueLineEdit->setEnabled(false);
    return;
  }
  const SplinePoint& p =
    GetSpline()->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
  mUI.valueLineEdit->setEnabled(true);
  mUI.valueLineEdit->setText(QString::number(p.mValue, 'f'));
}


void FloatSplineWatcher::UpdateRangeLabels() const
{
  const QString pointIndex =
    mSelectedPointIndex < 0 ? "-" : QString::number(mSelectedPointIndex);
  mUI.statusLabel->setText(QString("Point #%1, XView: %2, YCenter: %3, Zoom: (%4, %5)")
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

Vec2 FloatSplineWatcher::ScreenToPoint(QPoint& pos) const
{
  const Vec2 spp = GetStepsPerPixel();
  const float x = float(pos.x()) * spp.x + mLeftCenterPoint.x;
  const float y = mLeftCenterPoint.y -
    (0.5f * float(mSplineWidget->height()) - float(pos.y())) * spp.y;
  return Vec2(x, y);
}

Vec2 FloatSplineWatcher::GetStepsPerPixel() const
{
  return Vec2(powf(0.5f, mZoomLevel.x) / DefaultPixelsPerBeat,
              -powf(0.5f, mZoomLevel.y) / DefaultPixelsPerValue);
}

QPointF FloatSplineWatcher::GetPixelsPerStep() const
{
  return QPointF(powf(2.0f, mZoomLevel.x) * DefaultPixelsPerBeat,
                 -powf(2.0f, mZoomLevel.y) * DefaultPixelsPerValue);
}

static QPointF drawPoints[10000 * 2];


void FloatSplineWatcher::DrawSpline(QPaintEvent* ev) {
  QPainter painter(mSplineWidget);
  painter.fillRect(mSplineWidget->rect(), QBrush(QColor(53, 53, 53)));
  painter.setRenderHint(QPainter::Antialiasing);

  const float height = float(mSplineWidget->height());
  const float width = float(mSplineWidget->width());

  /// Draw center axes
  painter.setPen(QColor(80, 80, 80));
  const QPointF origo = ToScreenCoord(0, 0);
  painter.drawLine(QPointF(origo.x(), 0), QPointF(origo.x(), height));
  painter.drawLine(QPointF(0, origo.y()), QPointF(width, origo.y()));

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

  shared_ptr<FloatSplineNode> spline = GetSpline();

  /// Draw scene time
  painter.setPen(QColor(80, 200, 80));
  const QPointF timePoint = ToScreenCoord(spline->mTimeSlot.Get(), 0);
  painter.drawLine(QPointF(timePoint.x(), 0), QPointF(timePoint.x(), height));

  /// Draw spline
  painter.setPen(QColor(0, 192, 192));
  const UINT sampleCount = mSplineWidget->width();
  const Vec2 spp = GetStepsPerPixel();
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
  QPainter& painter, SplineLayer layer) {
  shared_ptr<FloatSplineNode> spline = GetSpline();
  SplineFloatComponent* component = spline->GetComponent(layer);
  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());

  const UINT sampleCount = mSplineWidget->width();
  const Vec2 spp = GetStepsPerPixel();
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
