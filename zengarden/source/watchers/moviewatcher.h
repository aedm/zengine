#pragma once

#include "watcherui.h"
#include <zengine.h>

class MovieWatcher: public WatcherUi {
public:
  MovieWatcher(const std::shared_ptr<Node>& node);
  virtual ~MovieWatcher();

  void OnRedraw() override;
  void OnTimeEdited(float time) override;
  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint();
  void HandleMovieCursorChange(float movieCursor) const;

  RenderTarget* mRenderTarget = nullptr;
};
