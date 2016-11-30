#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTextEdit>

class TextWatcher: public WatcherUI {
public:
  TextWatcher(StringNode* node, WatcherWidget* watcherWidget);
  virtual ~TextWatcher();

private:
  QBoxLayout*	mLayout;
  QTextEdit* mTextEdit;
};