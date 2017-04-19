#include "textwatcher.h"
#include <QtWidgets/QPushButton>

TextWatcher::TextWatcher(StringNode* node)
  : WatcherUI(node)
{
}

TextWatcher::~TextWatcher() {}

void TextWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  StringNode* stringNode = static_cast<StringNode*>(mNode);

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  mTextEdit = new QTextEdit(watcherWidget);
  mLayout->addWidget(mTextEdit);
  mTextEdit->setText(QString::fromStdString(stringNode->Get()));

  /// Recompile button
  QPushButton* compileButton = new QPushButton("Rebuild", watcherWidget);
  watcherWidget->connect(compileButton, &QPushButton::pressed, [=]() {
    stringNode->Set(mTextEdit->toPlainText().toStdString());
  });
  mLayout->addWidget(compileButton);
}
