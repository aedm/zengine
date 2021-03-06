#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>
#include "ui_movieeditor.h"

class TimelineEditor: public WatcherUi {
public:
  TimelineEditor(const std::shared_ptr<MovieNode>& movieNode);
  virtual ~TimelineEditor();

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;
  void OnRedraw() override;
  void OnChildNameChange() override;
  void SetSceneNodeForSelectedClip(const std::shared_ptr<SceneNode>& sceneNode) const;

private:
  Ui::MovieEditor mUI{};
  EventForwarderWidget* mTimelineCanvas{};

  void DrawTimeline(QPaintEvent* ev) const;

  float mTimelineStartTime = 0.0f;
  float mZoomLevel = 0.0f;

  enum class State {
    DEFAULT,
    WINDOW_MOVE,
    CLIP_MOVE,
    CLIP_LENGTH_ADJUST,
    TIME_SEEK,
  };
  State mState = State::DEFAULT;
  float mOriginalTimelineStartTime{};
  float mOriginalClipStart{};
  float mOriginalClipLength{};
  QPoint mOriginalMousePos;
  std::shared_ptr<ClipNode> mHoveredClip;
  std::shared_ptr<ClipNode> mSelectedClip;

  void HandleMovieCursorChange(float seconds) const;
  void HandleMouseDown(QMouseEvent* event);
  void HandleMouseUp(QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseMove(QMouseEvent* event);
  void HandleMouseWheel(QWheelEvent* event);

  int TimeToScreen(float time) const;
  int TimeRangeToPixels(float timeRange) const;
  float ScreenToTime(int xPos) const;
  float PixelsToTimeRange(int pixelCount) const;
};