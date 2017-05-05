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

private:
  Ui::MovieEditor mUI;


};