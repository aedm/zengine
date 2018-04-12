#include "textwatcher.h"
#include <QtWidgets/QPushButton>

TextWatcher::TextWatcher(const shared_ptr<StringNode>& node)
  : WatcherUI(node)
{}

TextWatcher::~TextWatcher() {}

void TextWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  shared_ptr<StringNode> stringNode = PointerCast<StringNode>(mNode);

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  mTextEdit = new QTextEdit(watcherWidget);
  mTextEdit->setCurrentFont(QFont("Consolas", 9));
  mLayout->addWidget(mTextEdit);
  mTextEdit->setText(QString::fromStdString(stringNode->Get()));

  /// Recompile button
  QPushButton* compileButton = new QPushButton("Rebuild", watcherWidget);
  watcherWidget->connect(compileButton, &QPushButton::pressed, [=]() {
    stringNode->Set(mTextEdit->toPlainText().toStdString());
  });
  mLayout->addWidget(compileButton);
}
