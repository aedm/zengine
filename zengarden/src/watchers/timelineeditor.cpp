#include "timelineeditor.h"
#include "../zengarden.h"
#include "../util/util.h"

static const float DefaultPixelsPerSecond = 100.0f;
static const int TrackHeightPixels = 20;

TimelineEditor::TimelineEditor(const std::shared_ptr<MovieNode>& movieNode)
  : WatcherUi(movieNode) 
{
  ZenGarden::GetInstance()->mOnMovieCursorChange += 
    Delegate(this, &TimelineEditor::HandleMovieCursorChange);
}

TimelineEditor::~TimelineEditor() {
  ZenGarden::GetInstance()->mOnMovieCursorChange -=
    Delegate(this, &TimelineEditor::HandleMovieCursorChange);
}

void TimelineEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);
  mUI.setupUi(watcherWidget);

  mUI.watchMovieButton->setFocusPolicy(Qt::NoFocus);
  mUI.timelineWidget->setFocusPolicy(Qt::NoFocus);

  QVBoxLayout* timelineLayout = new QVBoxLayout(mUI.timelineWidget);
  timelineLayout->setContentsMargins(0, 0, 0, 0);
  mTimelineCanvas = new EventForwarderWidget(mUI.timelineWidget);
  timelineLayout->addWidget(mTimelineCanvas);

  mTimelineCanvas->mOnPaint += Delegate(this, &TimelineEditor::DrawTimeline);
  mTimelineCanvas->mOnMousePress += Delegate(this, &TimelineEditor::HandleMouseDown);
  mTimelineCanvas->mOnMouseRelease += Delegate(this, &TimelineEditor::HandleMouseUp);
  mTimelineCanvas->mOnMouseMove += Delegate(this, &TimelineEditor::HandleMouseMove);
  mTimelineCanvas->mOnMouseWheel += Delegate(this, &TimelineEditor::HandleMouseWheel);

  QObject::connect(mUI.watchMovieButton, &QPushButton::pressed, [=]() {
    const std::shared_ptr<MovieNode> movieNode = PointerCast<MovieNode>(GetNode());
    ZenGarden::GetInstance()->Watch(movieNode, WatcherPosition::UPPER_LEFT_TAB);
  });
}

void TimelineEditor::OnRedraw() {
  mTimelineCanvas->update();
}

void TimelineEditor::OnChildNameChange() {
  mTimelineCanvas->update();
}

void TimelineEditor::SetSceneNodeForSelectedClip(const std::shared_ptr<SceneNode>& sceneNode) const
{
  if (!mSelectedClip) return;
  mSelectedClip->mSceneSlot.Connect(sceneNode);
}

void TimelineEditor::DrawTimeline(QPaintEvent* ev) const {
  QPainter painter(mTimelineCanvas);
  painter.fillRect(mTimelineCanvas->rect(), QBrush(QColor(23, 23, 23)));

  std::shared_ptr<MovieNode> movieNode = PointerCast<MovieNode>(GetNode());
  if (!movieNode) return;

  const float height = float(mTimelineCanvas->height());
  const float width = float(mTimelineCanvas->width());

  // Draw track separators
  painter.setPen(QColor(80, 80, 80));
  for (int i = 1; i <= movieNode->GetTrackCount(); i++) {
    const int y = TrackHeightPixels * i;
    painter.drawLine(QPoint(0, y), QPointF(width, y));
  }

  // Draw ruler
  painter.fillRect(QRect(0, 0, width, TrackHeightPixels), QBrush(QColor(120, 120, 120)));

  const float timeMarksInterval = powf(0.5f, roundf(mZoomLevel));
  const float firstMarkToDraw = 
    timeMarksInterval * floorf(mTimelineStartTime / timeMarksInterval);
  const float lastMarkToDraw = ScreenToTime(width);
  painter.setPen(QColor(0, 0, 0));
  for (float time = firstMarkToDraw; time < lastMarkToDraw; time += timeMarksInterval) {
    const int x = TimeToScreen(time);
    painter.drawLine(QPoint(x, TrackHeightPixels / 2), QPointF(x, TrackHeightPixels));
    painter.drawText(x + 5, TrackHeightPixels - 5, QString::number(time, 'f', 3));
  }

  // Draw tracks
  for (int i = 0; i < movieNode->GetTrackCount(); i++) {
    const int yTop = (i + 1) * TrackHeightPixels;
    const std::vector<std::shared_ptr<ClipNode>>& track = movieNode->GetTrack(i);
    for (const auto& clipNode : track) {
      const int left = TimeToScreen(clipNode->mStartTime.Get());
      int length = TimeRangeToPixels(clipNode->mLength.Get());
      if (length < 0) length = 0;
      QRect rect(left, yTop, length, TrackHeightPixels);

      if (clipNode == mSelectedClip) {
        painter.setBrush(QBrush(QColor(150, 50, 50)));
        painter.setPen(QColor(240, 240, 240));
      } else if (clipNode == mHoveredClip) {
        painter.setBrush(QBrush(QColor(150, 50, 50)));
        painter.setPen(QColor(160, 160, 160));
      } else {
        painter.setBrush(QBrush(QColor(120, 25, 25)));
        painter.setPen(QColor(128, 128, 128));
      }

      painter.drawRect(rect);

      painter.setPen(QColor(200, 200, 200));
      QRect textRect(left + 5, yTop, length - 5, TrackHeightPixels - 3);
      painter.drawText(textRect, Qt::AlignBottom | Qt::AlignLeft,
        QString::fromStdString(clipNode->GetName()));
    }
  }

  /// Draw movie cursor
  const float movieCursor = ZenGarden::GetInstance()->GetMovieCursor();
  const int cursorPos = TimeToScreen(movieCursor);
  painter.setPen(QColor(200, 200, 100));
  painter.drawLine(QPoint(cursorPos, 0), QPoint(cursorPos, height));
}

void TimelineEditor::HandleMovieCursorChange(float seconds) const
{
  mTimelineCanvas->update();
}

void TimelineEditor::HandleMouseDown(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void TimelineEditor::HandleMouseUp(QMouseEvent* event) {
  mState = State::DEFAULT;
}

void TimelineEditor::HandleMouseLeftDown(QMouseEvent* event) {
  switch (mState) {
    case State::DEFAULT:
      mSelectedClip = mHoveredClip;
      ZenGarden::GetInstance()->SetNodeForPropertyEditor(mSelectedClip);
      if (mSelectedClip) {
        mState = ((event->modifiers() & Qt::ShiftModifier) > 0)
          ? State::CLIP_LENGTH_ADJUST : State::CLIP_MOVE;
        mOriginalMousePos = event->pos();
        mOriginalClipStart = mSelectedClip->mStartTime.Get();
        mOriginalClipLength = mSelectedClip->mLength.Get();
      }
      else {
        mState = State::TIME_SEEK;
        ZenGarden::GetInstance()->SetMovieCursor(ScreenToTime(event->pos().x()));
      }
      mTimelineCanvas->update();
      break;
    default: break;
  }
}

void TimelineEditor::HandleMouseRightDown(QMouseEvent* event) {
  mState = State::WINDOW_MOVE;
  mOriginalTimelineStartTime = mTimelineStartTime;
  mOriginalMousePos = event->pos();
}

void TimelineEditor::HandleMouseMove(QMouseEvent* event) {
  switch (mState) {
    case State::DEFAULT:
    {
      std::shared_ptr<MovieNode> movieNode = PointerCast<MovieNode>(GetNode());
      if (!movieNode) break;
      const QPoint pos = event->pos();
      for (int i = 0; i < movieNode->GetTrackCount(); i++) {
        const int yTop = (i + 1) * TrackHeightPixels;
        const std::vector<std::shared_ptr<ClipNode>>& track = movieNode->GetTrack(i);
        for (const  auto& clipNode : track) {
          const int left = TimeToScreen(clipNode->mStartTime.Get());
          int length = TimeRangeToPixels(clipNode->mLength.Get());
          if (length < 0) length = 0;
          QRect rect(left, yTop, length, TrackHeightPixels);
          if (rect.contains(pos)) {
            if (mHoveredClip != clipNode) {
              mHoveredClip = clipNode;
              mTimelineCanvas->update();
            }
            return;
          }
        }
      }
      if (mHoveredClip) {
        mHoveredClip = nullptr;
        mTimelineCanvas->update();
      }
    }
    break;
    case State::TIME_SEEK:
      ZenGarden::GetInstance()->SetMovieCursor(ScreenToTime(event->pos().x()));
    break;
    case State::WINDOW_MOVE:
    {
      const QPoint diff = mOriginalMousePos - event->pos();
      const float timeDiff = PixelsToTimeRange(diff.x());
      mTimelineStartTime = mOriginalTimelineStartTime + timeDiff;
      mTimelineCanvas->update();
    }
    break;
    case State::CLIP_MOVE:
    {
      if (!mSelectedClip) return;
      const QPoint diff = mOriginalMousePos - event->pos();
      const float timeDiff = PixelsToTimeRange(diff.x());
      mSelectedClip->mStartTime.SetDefaultValue(mOriginalClipStart - timeDiff);
    }
    break;
    case State::CLIP_LENGTH_ADJUST:
    {
      if (!mSelectedClip) return;
      const QPoint diff = mOriginalMousePos - event->pos();
      const float timeDiff = PixelsToTimeRange(diff.x());
      mSelectedClip->mLength.SetDefaultValue(mOriginalClipLength - timeDiff);
    }
    break;
    default:
      break;
  }
}

void TimelineEditor::HandleMouseWheel(QWheelEvent* event) {
  const int zoomFactor = event->delta();
  const float timeUnderCursor = ScreenToTime(event->pos().x());
  mZoomLevel += float(zoomFactor) / 300.0f;
  const float newTimeUnderCursor = ScreenToTime(event->pos().x());
  mTimelineStartTime -= (newTimeUnderCursor - timeUnderCursor);
  mTimelineCanvas->update();
}

int TimelineEditor::TimeToScreen(float time) const
{
  return TimeRangeToPixels(time - mTimelineStartTime);
}

int TimelineEditor::TimeRangeToPixels(float timeRange) const
{
  // At zoomlevel 0 this is how many pixels should be one second
  const float pixelsPerSecond = DefaultPixelsPerSecond * powf(2.0f, mZoomLevel);
  return pixelsPerSecond * timeRange;
}

float TimelineEditor::ScreenToTime(int xPos) const
{
  return PixelsToTimeRange(xPos) + mTimelineStartTime;
}

float TimelineEditor::PixelsToTimeRange(int pixelCount) const
{
  const float pixelsPerSecond = DefaultPixelsPerSecond * powf(2.0f, mZoomLevel);
  return float(pixelCount) / pixelsPerSecond;
}

