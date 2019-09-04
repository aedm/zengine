#include "textwatcher.h"
#include <QtWidgets/QPushButton>

class CustomTextEdit : public QTextEdit {
public:
  CustomTextEdit(QWidget* parent)
    : QTextEdit(parent)
  {}

  void keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Return && (e->modifiers() & Qt::ControlModifier)) {
      mOnRebuild();
      return;
    }
    QTextEdit::keyPressEvent(e);
  }

  Event<> mOnRebuild;
};

TextWatcher::TextWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node)
{}

TextWatcher::~TextWatcher() {}

void TextWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  shared_ptr<StringNode> stringNode = PointerCast<StringNode>(GetNode());

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  auto textEdit = new CustomTextEdit(watcherWidget);
  mTextEdit = textEdit;
  mTextEdit->setCurrentFont(QFont("Consolas", 9));
  mLayout->addWidget(mTextEdit);
  mTextEdit->setText(QString::fromStdString(stringNode->Get()));

  textEdit->mOnRebuild += Delegate(this, &TextWatcher::HandleRebuid);

  /// Recompile button
  QPushButton* compileButton = new QPushButton("Rebuild", watcherWidget);
  watcherWidget->connect(compileButton, &QPushButton::pressed, [=]() {
    HandleRebuid();
  });
  mLayout->addWidget(compileButton);
}

void TextWatcher::HandleRebuid() {
  shared_ptr<StringNode> stringNode = PointerCast<StringNode>(GetNode());
  stringNode->Set(mTextEdit->toPlainText().toStdString());
}
