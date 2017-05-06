#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>
#include "ui_movieeditor.h"

class TimelineEditor: public WatcherUI {
public:
  TimelineEditor(MovieNode* movieNode);
  virtual ~TimelineEditor();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;
  virtual void OnRedraw() override;
  virtual void OnChildNameChange();

private:
  Ui::MovieEditor mUI;
  EventForwarderWidget* mTimelineCanvas;

  void DrawTimeline(QPaintEvent* ev);

  float mTimelineStartTime = 0.0f;
  float mZoomLevel = 0.0f;

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    CLIP_MOVE,
    CLIP_LENGTH_ADJUST,
  };
  State mState = State::DEFAULT;
  float mOriginalTimelineStartTime;
  float mOriginalClipStart;
  float mOriginalClipLength;
  QPoint mOriginalMousePos;
  ClipNode* mHoveredClip = nullptr;
  ClipNode* mSelectedClip = nullptr;

  void HandleMouseDown(QMouseEvent* event);
  void HandleMouseUp(QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseMove(QMouseEvent* event);
  void HandleMouseWheel(QWheelEvent* event);

  int TimeToScreen(float time);
  int TimeRangeToPixels(float timeRange);
  float ScreenToTime(int xPos);
  float PixelsToTimeRange(int pixelCount);
};