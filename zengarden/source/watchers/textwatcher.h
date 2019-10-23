#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QTextEdit>

class TextWatcher: public WatcherUi {
public:
  TextWatcher(const std::shared_ptr<Node>& node);
  virtual ~TextWatcher();

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  QBoxLayout* mLayout{};
  QTextEdit* mTextEdit{};

  void HandleRebuid() const;
};