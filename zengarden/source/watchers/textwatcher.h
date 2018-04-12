#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTextEdit>

class TextWatcher: public WatcherUI {
public:
  TextWatcher(const shared_ptr<StringNode>& node);
  virtual ~TextWatcher();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  QBoxLayout*	mLayout;
  QTextEdit* mTextEdit;
};