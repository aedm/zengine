#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class MovieWatcher: public WatcherUI {
public:
  MovieWatcher(Node* node);
  virtual ~MovieWatcher();

  virtual void OnRedraw() override;
  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(GLWidget* widget);

  RenderTarget* mRenderTarget = nullptr;

};
