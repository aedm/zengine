#include "splinewatcher.h"
#include <math.h>

template class SplineWatcher<NodeType::FLOAT>;

template<NodeType T>
SplineWatcher<T>::SplineWatcher(Node* node, WatcherWidget* watcherWidget)
  : Watcher(node, watcherWidget)
  , mXRange(0, 100)
  , mYRange(-5, 5) {
  mUI.setupUi(watcherWidget);

  QVBoxLayout* layout = new QVBoxLayout(mUI.splineFrame);
  layout->setContentsMargins(0, 0, 0, 0);

  mSplineWidget = new SplineWidget(mUI.splineFrame);
  layout->addWidget(mSplineWidget);
  mSplineWidget->mOnPaint += Delegate(this, &SplineWatcher<T>::DrawSpline);
  mSplineWidget->OnMousePress += Delegate(this, &SplineWatcher<T>::HandleMouseDown);
  mSplineWidget->OnMouseRelease += Delegate(this, &SplineWatcher<T>::HandleMouseUp);
  mSplineWidget->OnMouseMove += Delegate(this, &SplineWatcher<T>::HandleMouseMove);
  mSplineWidget->OnMouseWheel += Delegate(this, &SplineWatcher<T>::HandleMouseWheel);

  UpdateRangeLabels();
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
      SSpline* spline = dynamic_cast<SSpline*>(GetNode());
      spline->setPointValue(mHoveredPointIndex, xCursor, yCursor);
      mSplineWidget->update();
    } 
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
      SSpline* spline = dynamic_cast<SSpline*>(GetNode());
      int newIndex = -1;
      for (int i = 0; i < spline->getNumPoints(); i++) {
        const SSplinePoint& p = spline->getPoint(i);
        float x = (p.time - mXRange.x) * width / xDelta;
        float y = (p.value - mYRange.x) * height / yDelta;
        if (abs(x - xMouse) <= 5.0f && abs(y - yMouse) <= 5.0f) {
          newIndex = i;
          break;
        }
      }
      if (newIndex != mHoveredPointIndex) {
        mHoveredPointIndex = newIndex;
        mSplineWidget->update();
      }
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
  if (mHoveredPointIndex < 0) return;
  mState = State::POINT_MOVE;
  mOriginalMousePos = event->pos();

  SSpline* spline = dynamic_cast<SSpline*>(GetNode());
  const SSplinePoint& p = spline->getPoint(mHoveredPointIndex);
  mOriginalTime = p.time;
  mOriginalValue = p.value;
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
  xDelta *= zoom;
  yDelta *= zoom;
  mXRange.x = xCursor - float(event->pos().x()) * xDelta / width;
  mXRange.y = mXRange.x + xDelta;
  mYRange.x = yCursor - float(event->pos().y()) * yDelta / height;
  mYRange.y = mYRange.x + yDelta;
  mSplineWidget->update();
  UpdateRangeLabels();
}


template<NodeType T>
void SplineWatcher<T>::UpdateRangeLabels() {
  mUI.xRangeLabel->setText(QString("X: %1 .. %2")
                           .arg(QString::number(mXRange.x, 'f', 2), QString::number(mXRange.y, 'f', 2)));
  mUI.yRangeLabel->setText(QString("Y: %1 .. %2")
                           .arg(QString::number(mYRange.x, 'f', 2), QString::number(mYRange.y, 'f', 2)));
}


static QPointF drawPoints[10000 * 2];

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

  /// Draw spline
  painter.setPen(QColor(192, 192, 192));
  UINT sampleCount = mSplineWidget->width();
  SSpline* spline = dynamic_cast<SSpline*>(GetNode());
  float t = mXRange.x;
  float delta = (mXRange.y - mXRange.x) / float(sampleCount - 1);

  for (UINT i = 0; i < sampleCount; i++) {
    float splineVal = spline->getValue(t);
    float y = height * (splineVal - mYRange.x) / (mYRange.y - mYRange.x);
    drawPoints[i * 2] = QPointF(float(i), y);
    drawPoints[i * 2 + 1] = QPointF(float(i + 1), y);
    t += delta;
  }
  painter.drawLines(drawPoints + 1, sampleCount - 1);

  /// Draw control points
  painter.setPen(QColor(255, 255, 255));
  for (UINT i = 0; i < spline->getNumPoints(); i++) {
    painter.setBrush(i == mHoveredPointIndex ? QBrush(QColor(255, 255, 0)) : Qt::NoBrush);
    const SSplinePoint& point = spline->getPoint(i);
    float y = height * (point.value - mYRange.x) / (mYRange.y - mYRange.x);
    float x = width * (point.time - mXRange.x) / (mXRange.y - mXRange.x);
    painter.drawRect(QRectF(x - 4, y - 4, 8, 8));
  }
}



SplineWidget::SplineWidget(QWidget* parent)
  : QWidget(parent) {
  setMouseTracking(true);
}

void SplineWidget::paintEvent(QPaintEvent* ev) {
  mOnPaint(ev);
}

void SplineWidget::mouseMoveEvent(QMouseEvent* event) {
  OnMouseMove(event);
}

void SplineWidget::mousePressEvent(QMouseEvent* event) {
  OnMousePress(event);
}

void SplineWidget::mouseReleaseEvent(QMouseEvent* event) {
  OnMouseRelease(event);
}

void SplineWidget::keyPressEvent(QKeyEvent* event) {
  OnKeyPress(event);
}

void SplineWidget::keyReleaseEvent(QKeyEvent* event) {
  OnKeyRelease(event);
}

void SplineWidget::wheelEvent(QWheelEvent * event) {
  OnMouseWheel(event);
}
