#include "splinewatcher.h"
#include <math.h>
#include "../zengarden.h"

template class SplineWatcher<NodeType::FLOAT>;

template<NodeType T>
SplineWatcher<T>::SplineWatcher(Node* node)
  : WatcherUI(node)
  , mXRange(0, 10)
  , mYRange(5, -5) {}

template<NodeType T>
SplineWatcher<T>::~SplineWatcher() {}

template<NodeType T>
void SplineWatcher<T>::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);

  mUI.setupUi(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(mUI.splineFrame);
  layout->setContentsMargins(0, 0, 0, 0);

  mSplineWidget = new EventForwarderWidget(mUI.splineFrame);
  layout->addWidget(mSplineWidget);
  mSplineWidget->mOnPaint += Delegate(this, &SplineWatcher<T>::DrawSpline);
  mSplineWidget->OnMousePress += Delegate(this, &SplineWatcher<T>::HandleMouseDown);
  mSplineWidget->OnMouseRelease += Delegate(this, &SplineWatcher<T>::HandleMouseUp);
  mSplineWidget->OnMouseMove += Delegate(this, &SplineWatcher<T>::HandleMouseMove);
  mSplineWidget->OnMouseWheel += Delegate(this, &SplineWatcher<T>::HandleMouseWheel);

  UpdateRangeLabels();
  SelectPoint(SplineLayer::NONE, -1);

  mUI.addBasePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.addNoisePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.removePointButton->setFocusPolicy(Qt::NoFocus);
  mUI.linearCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.autotangentCheckBox->setFocusPolicy(Qt::NoFocus);
  mUI.breakpointCheckBox->setFocusPolicy(Qt::NoFocus);

  watcherWidget->connect(mUI.addBasePointButton, 
                         &QPushButton::pressed, [=]() { AddPoint(SplineLayer::BASE); });
  watcherWidget->connect(mUI.addNoisePointButton,
                         &QPushButton::pressed, [=]() { AddPoint(SplineLayer::NOISE); });
  watcherWidget->connect(
    mUI.removePointButton, &QPushButton::pressed, [=]() { RemovePoint(); });
  watcherWidget->connect(
    mUI.linearCheckBox, &QPushButton::pressed, [=]() { ToggleLinear(); });
  watcherWidget->connect(
    mUI.timeLineEdit, &QLineEdit::editingFinished, [=]() { HandleTimeEdited(); });
  watcherWidget->connect(
    mUI.valueLineEdit, &QLineEdit::editingFinished, [=]() { HandleValueEdited(); });
}


template<NodeType T>
void SplineWatcher<T>::OnRedraw() {
  mSplineWidget->update();
}


template<NodeType T>
void SplineWatcher<T>::OnSplineControlPointsChanged() {
  SelectPoint(mSelectedLayer, mSelectedPointIndex);
}


template<NodeType T>
void SplineWatcher<T>::AddPoint(SplineLayer layer) {
  FloatSplineNode* spline = GetSpline();
  float time = spline->mTimeSlot.Get();
  float value = spline->GetComponent(layer)->Get(time);
  int index = spline->AddPoint(layer, time, value);
  mSelectedPointIndex = -1;
  SelectPoint(layer, index);
}


template<NodeType T>
void SplineWatcher<T>::RemovePoint() {
  if (mSelectedPointIndex >= 0) {
    GetSpline()->RemovePoint(mSelectedLayer, mSelectedPointIndex);
  }
}

template<NodeType T>
void SplineWatcher<T>::ToggleLinear() {
  if (mSelectedPointIndex >= 0 && mSelectedLayer == SplineLayer::BASE) {
    auto& points = GetSpline()->GetComponent(mSelectedLayer)->GetPoints();
    GetSpline()->SetLinear(mSelectedLayer, mSelectedPointIndex, 
                           !points[mSelectedPointIndex].mIsLinear);
  }
}


template<NodeType T>
void SplineWatcher<T>::HandleValueEdited() {
  if (mSelectedPointIndex < 0) return;
  QString uiString = mUI.valueLineEdit->text();
  bool ok;
  float f = uiString.toFloat(&ok);
  if (ok) {
    FloatSplineNode* spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, point.mTime, f);
    mUI.valueLineEdit->clearFocus();
  }
}


template<NodeType T>
void SplineWatcher<T>::HandleTimeEdited() {
  if (mSelectedPointIndex < 0) return;
  QString uiString = mUI.timeLineEdit->text();
  bool ok;
  float f = uiString.toFloat(&ok);
  if (ok) {
    FloatSplineNode* spline = GetSpline();
    auto& point = spline->GetComponent(mSelectedLayer)->GetPoints()[mSelectedPointIndex];
    spline->SetPointValue(mSelectedLayer, mSelectedPointIndex, f, point.mValue);
    mUI.timeLineEdit->clearFocus();
  }
}


template<NodeType T>
FloatSplineNode* SplineWatcher<T>::GetSpline() {
  return SafeCast<FloatSplineNode*>(GetNode());
}


template<NodeType T>
void SplineWatcher<T>::OnTimeEdited(float time) {
  mSplineWidget->update();
}


template<NodeType T>
void SplineWatcher<T>::HandleMouseMove(QMouseEvent* event) {
  QSize size = mSplineWidget->size();
  switch (mState) {
    case State::WINDOW_MOVE:
    {
      QPoint diff = mOriginalMousePos - event->pos();
      float dx = (mXRangeOriginal.y - mXRangeOriginal.x) * float(diff.x()) / float(size.width());
      float dy = (mYRangeOriginal.y - mYRangeOriginal.x) * float(diff.y()) / float(size.height());
      mXRange = mXRangeOriginal + Vec2(dx, dx);
      mYRange = mYRangeOriginal + Vec2(dy, dy);
      mSplineWidget->update();
      UpdateRangeLabels();
    }
    break;
    case State::POINT_MOVE:
    {
      float height = float(mSplineWidget->height());
      float width = float(mSplineWidget->width());
      float xDelta = mXRange.y - mXRange.x;
      float yDelta = mYRange.y - mYRange.x;
      QPoint diff = event->pos() - mOriginalMousePos;
      float xCursor = mOriginalTime + float(diff.x()) * xDelta / width;
      float yCursor = mOriginalValue + float(diff.y()) * yDelta / height;
      FloatSplineNode* spline = dynamic_cast<FloatSplineNode*>(GetNode());
      spline->SetPointValue(mHoveredLayer, mHoveredPointIndex, xCursor, yCursor);
      mSplineWidget->update();
      SelectPoint(mSelectedLayer, mSelectedPointIndex);
    }
    break;
    case SplineWatcher::State::TIME_MOVE:
      GetSpline()->mSceneTimeNode.EditTime(ScreenToTime(event->pos().x()));
      break;
    default:
    {
      float height = float(mSplineWidget->height());
      float width = float(mSplineWidget->width());
      float xDelta = mXRange.y - mXRange.x;
      float yDelta = mYRange.y - mYRange.x;
      float xCursor = mXRange.x + float(event->pos().x()) * xDelta / width;
      float yCursor = mYRange.x + float(event->pos().y()) * yDelta / height;
      float xMouse = float(event->pos().x());
      float yMouse = float(event->pos().y());
      FloatSplineNode* spline = GetSpline();

      for (UINT layer = UINT(SplineLayer::BASE); layer < UINT(SplineLayer::COUNT); 
           layer++) {
        auto& points = spline->GetComponent(SplineLayer(layer))->GetPoints();
        for (int i = 0; i < points.size(); i++) {
          const SplinePoint& p = points[i];
          float x = (p.mTime - mXRange.x) * width / xDelta;
          float y = (p.mValue - mYRange.x) * height / yDelta;
          if (abs(x - xMouse) <= 5.0f && abs(y - yMouse) <= 5.0f) {
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

template<NodeType T>
void SplineWatcher<T>::HandleMouseDown(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

template<NodeType T>
void SplineWatcher<T>::HandleMouseRightDown(QMouseEvent* event) {
  mState = State::WINDOW_MOVE;
  mXRangeOriginal = mXRange;
  mYRangeOriginal = mYRange;
  mOriginalMousePos = event->pos();
}

template<NodeType T>
void SplineWatcher<T>::HandleMouseLeftDown(QMouseEvent* event) {
  if (mHoveredPointIndex >= 0) {
    /// Select point
    mState = State::POINT_MOVE;
    SelectPoint(mHoveredLayer, mHoveredPointIndex);
    mOriginalMousePos = event->pos();

    FloatSplineNode* spline = GetSpline();
    const SplinePoint& p = 
      spline->GetComponent(mHoveredLayer)->GetPoints()[mHoveredPointIndex];
    mOriginalTime = p.mTime;
    mOriginalValue = p.mValue;
  } else {
    /// Move time
    mState = SplineWatcher::State::TIME_MOVE;
    GetSpline()->mSceneTimeNode.EditTime(ScreenToTime(event->pos().x()));
    SelectPoint(SplineLayer::NONE, -1);
  }
}


template<NodeType T>
void SplineWatcher<T>::HandleMouseUp(QMouseEvent* event) {
  switch (mState) {
    case State::WINDOW_MOVE:
      break;
    default: break;
  }
  mState = State::DEFAULT;
}


template<NodeType T>
void SplineWatcher<T>::HandleMouseWheel(QWheelEvent* event) {
  int zoomFactor = -event->delta();
  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());
  float xDelta = mXRange.y - mXRange.x;
  float yDelta = mYRange.y - mYRange.x;
  float xCursor = mXRange.x + float(event->pos().x()) * xDelta / width;
  float yCursor = mYRange.x + float(event->pos().y()) * yDelta / height;

  float zoom = powf(2.0, float(zoomFactor) / (120.0f * 4.0f));
  if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
    yDelta *= zoom;
    mYRange.x = yCursor - float(event->pos().y()) * yDelta / height;
    mYRange.y = mYRange.x + yDelta;
  } else {
    xDelta *= zoom;
    mXRange.x = xCursor - float(event->pos().x()) * xDelta / width;
    mXRange.y = mXRange.x + xDelta;
  }
  mSplineWidget->update();
  UpdateRangeLabels();
}


template<NodeType T>
void SplineWatcher<T>::SelectPoint(SplineLayer layer, int index) {
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



template<NodeType T>
void SplineWatcher<T>::UpdateTimeEdit() {
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


template<NodeType T>
void SplineWatcher<T>::UpdateValueEdit() {
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


template<NodeType T>
void SplineWatcher<T>::UpdateRangeLabels() {
  QString pointIndex =
    mSelectedPointIndex < 0 ? "-" : QString::number(mSelectedPointIndex);
  mUI.statusLabel->setText(QString("Point #%1, X: [%2..%3], Y: [%4..%5]")
                           .arg(pointIndex,
                                QString::number(mXRange.x, 'f', 2),
                                QString::number(mXRange.y, 'f', 2),
                                QString::number(mYRange.x, 'f', 2),
                                QString::number(mYRange.y, 'f', 2)));
}


template<NodeType T>
QPointF SplineWatcher<T>::ToScreenCoord(float time, float value) {
  QSize size = mSplineWidget->size();
  float xCoord = (time - mXRange.x) * float(size.width()) / (mXRange.y - mXRange.x);
  float yCoord = (value - mYRange.x) * float(size.height()) / (mYRange.y - mYRange.x);
  return QPointF(xCoord, yCoord);
}


template<NodeType T>
float SplineWatcher<T>::ScreenToTime(int xPos) {
  return mXRange.x + (mXRange.y - mXRange.x) * float(xPos) / float(mSplineWidget->width());
}

static QPointF drawPoints[10000 * 2];

// TODO: temp, remove it
const float gBPM = 140.0f;
const float gBeatTime = 60.0f / gBPM;
const float gSection = 1.0 * gBeatTime;

template<>
void SplineWatcher<NodeType::FLOAT>::DrawSpline(QPaintEvent* ev) {
  QPainter painter(mSplineWidget);
  painter.fillRect(mSplineWidget->rect(), QBrush(QColor(53, 53, 53)));
  painter.setRenderHint(QPainter::Antialiasing);

  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());

  /// Draw center axes
  painter.setPen(QColor(80, 80, 80));
  float xCenter = -mXRange.x * (width / (mXRange.y - mXRange.x));
  float yCenter = -mYRange.x * (height / (mYRange.y - mYRange.x));
  painter.drawLine(QPointF(xCenter, 0), QPointF(xCenter, height));
  painter.drawLine(QPointF(0, yCenter), QPointF(width, yCenter));

  /// Draw beats
  int beat = int(ScreenToTime(0) / gSection) - 1;
  while (true) {
    float beatCoord = ToScreenCoord(float(beat) * gSection, 0).x();
    if (beatCoord > width) break;
    painter.setPen(beat % 64 == 0 ? QColor(150, 150, 0)
                   : beat % 16 == 0 ? QColor(160, 40, 40)
                   : beat % 4 == 0 ? QColor(0, 0, 0)
                   : QColor(40, 40, 40));
    painter.drawLine(QPointF(beatCoord, 0), QPointF(beatCoord, height));
    beat++;
  }

  FloatSplineNode* spline = GetSpline();

  /// Draw scene time
  painter.setPen(QColor(80, 200, 80));
  QPointF timePoint = ToScreenCoord(spline->mTimeSlot.Get(), 0);
  painter.drawLine(QPointF(timePoint.x(), 0), QPointF(timePoint.x(), height));

  /// Draw spline
  painter.setPen(QColor(192, 192, 192));
  UINT sampleCount = mSplineWidget->width();
  float t = mXRange.x;
  float delta = (mXRange.y - mXRange.x) / float(sampleCount - 1);

  for (UINT i = 0; i < sampleCount; i++) {
    float splineVal = spline->GetValue(t);
    float y = height * (splineVal - mYRange.x) / (mYRange.y - mYRange.x);
    drawPoints[i * 2] = QPointF(float(i), y);
    drawPoints[i * 2 + 1] = QPointF(float(i), y);
    t += delta;
  }
  painter.drawLines(drawPoints + 1, sampleCount - 1);

  /// Draw spline components
  painter.setPen(QColor(0, 192, 192));
  DrawSplineComponentControl(painter, SplineLayer::BASE);

  painter.setPen(QColor(192, 192, 0));
  DrawSplineComponentControl(painter, SplineLayer::NOISE);
}


template<>
void SplineWatcher<NodeType::FLOAT>::DrawSplineComponentControl(
  QPainter& painter, SplineLayer layer) 
{
  FloatSplineNode* spline = GetSpline();
  SplineFloatComponent* component = spline->GetComponent(layer);
  float height = float(mSplineWidget->height());
  float width = float(mSplineWidget->width());

  UINT sampleCount = mSplineWidget->width();
  float t = mXRange.x;
  float delta = (mXRange.y - mXRange.x) / float(sampleCount - 1);

  for (UINT i = 0; i < sampleCount; i++) {
    float splineVal = component->Get(t);
    float y = height * (splineVal - mYRange.x) / (mYRange.y - mYRange.x);
    drawPoints[i * 2] = QPointF(float(i), y);
    drawPoints[i * 2 + 1] = QPointF(float(i), y);
    t += delta;
  }
  painter.drawLines(drawPoints + 1, sampleCount - 1);

  auto& points = component->GetPoints();

  /// Draw control points
  painter.setPen(QColor(255, 255, 255));
  for (UINT i = 0; i < points.size(); i++) {
    painter.setBrush((i == mSelectedPointIndex && layer == mSelectedLayer)
                     ? QBrush(QColor(0, 255, 255))
                     : (i == mHoveredPointIndex && layer == mHoveredLayer) 
                     ? QBrush(QColor(255, 255, 0)) : Qt::NoBrush);
    const SplinePoint& point = points[i];
    float y = height * (point.mValue - mYRange.x) / (mYRange.y - mYRange.x);
    float x = width * (point.mTime - mXRange.x) / (mXRange.y - mXRange.x);
    painter.drawRect(QRectF(x - 4, y - 4, 8, 8));
  }
}
