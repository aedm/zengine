#include "propertyeditor.h"
#include "../graph/prototypes.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QPushButton>

PropertyEditor::PropertyEditor(const shared_ptr<Node>& node)
  : WatcherUI(node) {}


void PropertyEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  shared_ptr<Node> node = GetDirectNode();

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  /// Node type
  shared_ptr<Node> referencedNode = node->GetReferencedNode();
  string typeString;
  if (referencedNode.use_count() > 0) {
    typeString =
      NodeRegistry::GetInstance()->GetNodeClass(node->GetReferencedNode())->mClassName;
  }
  else typeString = "Ghost";
  QLabel* typeLabel = new QLabel(QString::fromStdString(typeString), watcherWidget);
  typeLabel->setAlignment(Qt::AlignHCenter);
  QFont font = typeLabel->font();
  font.setBold(true);
  typeLabel->setFont(font);
  mLayout->addWidget(typeLabel);

  /// Name input
  QWidget* nameEditor = new QWidget(watcherWidget);
  QHBoxLayout* nameEditorLayout = new QHBoxLayout(nameEditor);
  nameEditorLayout->setSpacing(6);
  nameEditorLayout->setContentsMargins(0, 0, 0, 0);
  mNameTextBox = new TextBox(nameEditor);
  mNameTextBox->setPlaceholderText(QString("noname"));
  if (!node->GetName().empty()) {
    mNameTextBox->setText(QString::fromStdString(node->GetName()));
  }
  mNameTextBox->onEditingFinished +=
    Delegate(this, &PropertyEditor::HandleNameTexBoxChanged);
  nameEditorLayout->addWidget(mNameTextBox);
  mLayout->addWidget(nameEditor);
}


void PropertyEditor::HandleNameTexBoxChanged() {
  shared_ptr<Node> node = GetDirectNode();
  if (node) {
    node->SetName(mNameTextBox->text().toStdString());
  }
}


template<ValueType T>
StaticValueWatcher<T>::StaticValueWatcher(const shared_ptr<StaticValueNode<T>>& node)
  : PropertyEditor(node) {}


template<ValueType T>
void StaticValueWatcher<T>::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  shared_ptr<StaticValueNode<T>> vectorNode = PointerCast<StaticValueNode<T>>(GetNode());
  mVectorEditor = new ValueEditor<T>(watcherWidget, "", vectorNode->Get());
  mVectorEditor->onValueChange +=
    Delegate(this, &StaticValueWatcher<T>::HandleEditorValueChange);
  mLayout->addWidget(mVectorEditor);
}


template<ValueType T>
void StaticValueWatcher<T>::HandleEditorValueChange(QWidget* editor,
  const VectorType& value)
{
  shared_ptr<StaticValueNode<T>> vectorNode = PointerCast<StaticValueNode<T>>(GetNode());
  vectorNode->Set(value);
}


template class StaticValueWatcher<ValueType::FLOAT>;
template class StaticValueWatcher<ValueType::VEC2>;
template class StaticValueWatcher<ValueType::VEC3>;
template class StaticValueWatcher<ValueType::VEC4>;
