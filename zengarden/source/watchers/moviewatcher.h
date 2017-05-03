#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>
#include "ui_movieeditor.h"

class MovieWatcher: public WatcherUI {
public:
  MovieWatcher(MovieNode* movieNode);
  virtual ~MovieWatcher();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  Ui::MovieEditor mUI;


};