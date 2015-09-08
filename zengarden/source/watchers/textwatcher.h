#pragma once

#include "watcher.h"
#include "watcherwidget.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTextEdit>

class TextWatcher: public Watcher {
public:
  TextWatcher(StringNode* node, WatcherWidget* watcherWidget);
  virtual ~TextWatcher();

private:
  QBoxLayout*	mLayout;
  QTextEdit* mTextEdit;
};