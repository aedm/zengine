#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class MovieWatcher: public WatcherUI {
public:
  MovieWatcher(const shared_ptr<Node>& node);
  virtual ~MovieWatcher();

  virtual void OnRedraw() override;
  virtual void OnTimeEdited(float time) override;
  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(EventForwarderGLWidget* widget);
  void HandleMovieCursorChange(float movieCursor);

  RenderTarget* mRenderTarget = nullptr;
};
