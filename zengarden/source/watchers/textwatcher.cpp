#include "textwatcher.h"
#include <QtWidgets/QPushButton>

TextWatcher::TextWatcher(StringNode* node, WatcherWidget* watcherWidget)
  : WatcherUI(node, watcherWidget)
{
  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  mTextEdit = new QTextEdit(watcherWidget);
  mLayout->addWidget(mTextEdit);
  mTextEdit->setText(QString::fromStdString(node->Get()));

  /// Recompile button
  QPushButton* compileButton = new QPushButton("Rebuild", watcherWidget);
  watcherWidget->connect(compileButton, &QPushButton::pressed, [=]() {
     node->Set(mTextEdit->toPlainText().toStdString());
  });
  mLayout->addWidget(compileButton);

}

TextWatcher::~TextWatcher() {

}
