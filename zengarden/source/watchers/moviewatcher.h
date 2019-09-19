#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class MovieWatcher: public WatcherUI {
public:
  MovieWatcher(const shared_ptr<Node>& node);
  virtual ~MovieWatcher();

  void OnRedraw() override;
  void OnTimeEdited(float time) override;
  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(EventForwarderGLWidget* widget);
  void HandleMovieCursorChange(float movieCursor) const;

  RenderTarget* mRenderTarget = nullptr;
};
