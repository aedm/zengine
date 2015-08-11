#include "parameterwidgets.h"
#include "../watchers/watcherwidget.h"
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/qgridlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qgroupbox.h>
#include <QtWidgets/qradiobutton.h>
#include <QtWidgets/qbuttongroup.h>
#include <QtGui/qpainter.h>
#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QTime>


Slider::Slider(QWidget* Parent, QString text, float value, float minimum, float maximum)
  : QWidget(Parent)
  , mValue(value)
  , mMinimum(minimum)
  , mMaximum(maximum)
  , mText(text) 
  , mIsReadOnly(false)
{}

void Slider::paintEvent(QPaintEvent *e) {
  QPainter painter(this);
  QSize widgetSize = size();
  painter.setPen(palette().dark().color().darker());
  painter.setBrush(palette().alternateBase().color());
  painter.drawRect(0, 0, widgetSize.width() - 1, widgetSize.height() - 1);

  if (mMaximum > mMinimum) {
    painter.setPen(Qt::NoPen);
    if (mIsReadOnly) painter.setBrush(palette().highlight().color().darker());
    else painter.setBrush(palette().highlight().color());

    int widthPx = widgetSize.width() - 2;
    int full = float(widthPx) * mValue / (mMaximum - mMinimum);
    if (full > widthPx) full = widthPx;
    else if (full < 0) full = 0;
    painter.drawRect(1, 1, full, widgetSize.height() - 2);
  }

  if (!mText.isEmpty()) {
    painter.setPen(palette().windowText().color());
    painter.drawText(5, 0, width(), height(), Qt::AlignVCenter, mText);
  }
}

void Slider::mousePressEvent(QMouseEvent * event) {
  HandleMouse(event);
}

void Slider::mouseMoveEvent(QMouseEvent * event) {
  if (event->buttons() & Qt::LeftButton) {
    HandleMouse(event);
  }
}

void Slider::HandleMouse(QMouseEvent * event) {
  if (mMaximum > mMinimum) {
    float newValue = float(event->x() + 1) * (mMaximum - mMinimum) / float(width());
    if (mValue != newValue) {
      onValueChange(newValue);
    }
  }
}

float Slider::Get() {
  return mValue;
}

void Slider::Set(float value) {
  if (value != mValue) {
    this->mValue = value;
    update();
  }
}

void Slider::SetReadOnly(bool readOnly) {
  if (mIsReadOnly != readOnly) {
    mIsReadOnly = readOnly;
    update();
  }
}


TextBox::TextBox(QWidget* parent)
  : QLineEdit(parent) {
  connect(this, SIGNAL(editingFinished()), this, SLOT(HandleEditingFinished()));
}


void TextBox::HandleEditingFinished() {
  clearFocus();
  onEditingFinished();
}


FloatEditor::FloatEditor(QWidget* parent, QString name, float value)
  : QWidget(parent)
  , mValue(value) {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  mSlider = new Slider(this, name, mValue, 0, 1);
  mSlider->setMinimumHeight(20);
  mSlider->setMinimumWidth(70);
  mSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  mSlider->Set(value);
  layout->addWidget(mSlider);

  mTextBox = new TextBox(this);
  SetTextBoxValue(value);
  mTextBox->setFixedWidth(40);
  layout->addWidget(mTextBox);

  mSlider->onValueChange += Delegate(this, &FloatEditor::SliderValueChanged);
  mTextBox->onEditingFinished += Delegate(this, &FloatEditor::SpinBoxValueChanged);
}


void FloatEditor::SetTextBoxValue(float value) {
  if (mAllowTextboxValueChanges) {
    mTextBox->setText(QString::number(value, 'g', 3));
  }
}


void FloatEditor::SliderValueChanged(float value) {
  if (mIsReadOnly || value == mValue) return;
  mValue = value;
  SetTextBoxValue(value);
  /// TODO: slider should set and repaint itself
  mSlider->Set(value);
  onValueChange(this, value);
}


void FloatEditor::SpinBoxValueChanged() {
  float value = mTextBox->text().toFloat();
  if (mIsReadOnly || value == mValue) return;
  /// TODO: slider should set and repaint itself
  mSlider->Set(value);
  mAllowTextboxValueChanges = false;
  onValueChange(this, value);
  mAllowTextboxValueChanges = true;
}


void FloatEditor::Set(float value) {
  if (mIsReadOnly || value == mValue) return;
  mValue = value;
  SetTextBoxValue(value);
  mSlider->Set(value);
}

void FloatEditor::SetReadOnly(bool readOnly) {
  mSlider->SetReadOnly(readOnly);
  mTextBox->setReadOnly(readOnly);
  mIsReadOnly = readOnly;
}


FloatWatcher::FloatWatcher(ValueNode<NodeType::FLOAT>* node, WatcherWidget* widget, 
                           QString name, bool readOnly)
  : Watcher(node, widget)
{
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  mEditorX = new FloatEditor(widget, name, node->Get());
  mEditorX->onValueChange += Delegate(this, &FloatWatcher::HandleValueChange);
  layout->addWidget(mEditorX);

  SetReadOnly(readOnly);
}


void FloatWatcher::HandleSniffedMessage(NodeMessage message, Slot*, void* payload) {
  switch (message) {
    case NodeMessage::VALUE_CHANGED: {
      float value = static_cast<ValueNode<NodeType::FLOAT>*>(mNode)->Get();
      mEditorX->Set(value);
      break;
    }
    default: break;
  }
}


void FloatWatcher::SetReadOnly(bool readOnly) {
  mEditorX->SetReadOnly(readOnly);
}


void FloatWatcher::HandleChangedNode(Node* node) {
  float value = node ? static_cast<ValueNode<NodeType::FLOAT>*>(node)->Get() : 0.0f;
  mEditorX->Set(value);
}


void FloatWatcher::HandleValueChange(FloatEditor* editor, float value) {
  if (mNode == nullptr) return;
  ASSERT(dynamic_cast<FloatNode*>(mNode) != nullptr);
  FloatNode* node = static_cast<FloatNode*>(mNode);
  node->Set(value);
}



Vec3Watcher::Vec3Watcher(ValueNode<NodeType::VEC3>* node, WatcherWidget* widget,
                         QString name, bool readOnly)
  : Watcher(node, widget) 
{
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  Vec3 value = node->Get();

  mEditorX = new FloatEditor(widget, name + ".x", value.x);
  mEditorX->onValueChange += Delegate(this, &Vec3Watcher::HandleValueChange);
  layout->addWidget(mEditorX);

  mEditorY = new FloatEditor(widget, name + ".y", value.y);
  mEditorY->onValueChange += Delegate(this, &Vec3Watcher::HandleValueChange);
  layout->addWidget(mEditorY);

  mEditorZ = new FloatEditor(widget, name + ".z", value.z);
  mEditorZ->onValueChange += Delegate(this, &Vec3Watcher::HandleValueChange);
  layout->addWidget(mEditorZ);

  SetReadOnly(readOnly);
}


void Vec3Watcher::HandleSniffedMessage(NodeMessage message, Slot*, void* payload) {
  switch (message) {
    case NodeMessage::VALUE_CHANGED: {
      Vec3 value = static_cast<ValueNode<NodeType::VEC3>*>(mNode)->Get();
      mEditorX->Set(value.x);
      mEditorY->Set(value.y);
      mEditorZ->Set(value.z);
      break;
    }
    default: break;
  }
}


void Vec3Watcher::SetReadOnly(bool readOnly) {
  mEditorX->SetReadOnly(readOnly);
  mEditorY->SetReadOnly(readOnly);
  mEditorZ->SetReadOnly(readOnly);
}


void Vec3Watcher::HandleChangedNode(Node* node) {
  Vec3 value = 
    node ? static_cast<ValueNode<NodeType::VEC3>*>(node)->Get() : Vec3(0, 0, 0);
  mEditorX->Set(value.x);
  mEditorY->Set(value.y);
  mEditorZ->Set(value.z);
}


void Vec3Watcher::HandleValueChange(FloatEditor* editor, float value) {
  if (mNode == nullptr) return;
  ASSERT(dynamic_cast<Vec3Node*>(mNode) != nullptr);
  Vec3Node* node = static_cast<Vec3Node*>(mNode);
  Vec3 v = node->Get();
  if (editor == mEditorX) v.x = value;
  else if (editor == mEditorY) v.y = value;
  else if (editor == mEditorZ) v.z = value;
  node->Set(v);
}



Vec4Watcher::Vec4Watcher(ValueNode<NodeType::VEC4>* node, WatcherWidget* widget,
                         QString name, bool readOnly)
  : Watcher(node, widget) 
{
  QVBoxLayout* layout = new QVBoxLayout(widget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  Vec4 value = node->Get();

  mEditorX = new FloatEditor(widget, name + ".x", value.x);
  mEditorX->onValueChange += Delegate(this, &Vec4Watcher::HandleValueChange);
  layout->addWidget(mEditorX);

  mEditorY = new FloatEditor(widget, name + ".y", value.y);
  mEditorY->onValueChange += Delegate(this, &Vec4Watcher::HandleValueChange);
  layout->addWidget(mEditorY);

  mEditorZ = new FloatEditor(widget, name + ".z", value.z);
  mEditorZ->onValueChange += Delegate(this, &Vec4Watcher::HandleValueChange);
  layout->addWidget(mEditorZ);

  mEditorW = new FloatEditor(widget, name + ".w", value.w);
  mEditorW->onValueChange += Delegate(this, &Vec4Watcher::HandleValueChange);
  layout->addWidget(mEditorW);

  SetReadOnly(readOnly);
}


void Vec4Watcher::HandleSniffedMessage(NodeMessage message, Slot*, void* payload) {
  switch (message) {
    case NodeMessage::VALUE_CHANGED: {
      Vec4 value = static_cast<ValueNode<NodeType::VEC4>*>(mNode)->Get();
      mEditorX->Set(value.x);
      mEditorY->Set(value.y);
      mEditorZ->Set(value.z);
      mEditorW->Set(value.w);
      break;
    }
    default: break;
  }
}


void Vec4Watcher::SetReadOnly(bool readOnly) {
  mEditorX->SetReadOnly(readOnly);
  mEditorY->SetReadOnly(readOnly);
  mEditorZ->SetReadOnly(readOnly);
  mEditorW->SetReadOnly(readOnly);
}


void Vec4Watcher::HandleChangedNode(Node* node) {
  Vec4 value =
    node ? static_cast<ValueNode<NodeType::VEC4>*>(node)->Get() : Vec4(0, 0, 0, 0);
  mEditorX->Set(value.x);
  mEditorY->Set(value.y);
  mEditorZ->Set(value.z);
  mEditorW->Set(value.w);
}


void Vec4Watcher::HandleValueChange(FloatEditor* editor, float value) {
  if (mNode == nullptr) return;
  ASSERT(dynamic_cast<Vec4Node*>(mNode) != nullptr);
  Vec4Node* node = static_cast<Vec4Node*>(mNode);
  Vec4 v = node->Get();
  if (editor == mEditorX) v.x = value;
  else if (editor == mEditorY) v.y = value;
  else if (editor == mEditorZ) v.z = value;
  else if (editor == mEditorW) v.w = value;
  node->Set(v);
}