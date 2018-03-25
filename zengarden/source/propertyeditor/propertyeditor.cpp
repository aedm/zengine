#include "propertyeditor.h"
#include "../graph/prototypes.h"
#include "../watchers/watcherwidget.h"
#include "../zengarden.h"
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QPushButton>

PropertyEditor::PropertyEditor(Node* node)
  : WatcherUI(node) {}


void PropertyEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);

  /// Vertical layout
  mLayout = new QVBoxLayout(watcherWidget);
  mLayout->setSpacing(4);
  mLayout->setContentsMargins(0, 0, 0, 0);

  /// Node type
  string& typeString = NodeRegistry::GetInstance()->GetNodeClass(mNode->GetReferencedNode())->mClassName;
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
  if (!mNode->GetName().empty()) {
    mNameTextBox->setText(QString::fromStdString(mNode->GetName()));
  }
  mNameTextBox->onEditingFinished +=
    Delegate(this, &PropertyEditor::HandleNameTexBoxChanged);
  nameEditorLayout->addWidget(mNameTextBox);
  mLayout->addWidget(nameEditor);
}


void PropertyEditor::HandleNameTexBoxChanged() {
  if (mNode) {
    mNode->SetName(mNameTextBox->text().toStdString());
  }
}


template<ValueType T>
StaticValueWatcher<T>::StaticValueWatcher(StaticValueNode<T>* node)
  : PropertyEditor(node) {}


template<ValueType T>
void StaticValueWatcher<T>::SetWatcherWidget(WatcherWidget* watcherWidget) {
  PropertyEditor::SetWatcherWidget(watcherWidget);

  StaticValueNode<T>* vectorNode = SafeCast<StaticValueNode<T>*>(mNode);
  mVectorEditor = new ValueEditor<T>(watcherWidget, "", vectorNode->Get());
  mVectorEditor->onValueChange +=
    Delegate(this, &StaticValueWatcher<T>::HandleEditorValueChange);
  mLayout->addWidget(mVectorEditor);
}


template<ValueType T>
void StaticValueWatcher<T>::HandleEditorValueChange(QWidget* editor,
  const VectorType& value) 
{
  StaticValueNode<T>* vectorNode = SafeCast<StaticValueNode<T>*>(mNode);
  vectorNode->Set(value);
}


template class StaticValueWatcher<ValueType::FLOAT>;
template class StaticValueWatcher<ValueType::VEC2>;
template class StaticValueWatcher<ValueType::VEC3>;
template class StaticValueWatcher<ValueType::VEC4>;
