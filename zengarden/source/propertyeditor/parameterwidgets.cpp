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
    repaint();
  }
}

void Slider::SetReadOnly(bool readOnly) {
  if (mIsReadOnly != readOnly) {
    mIsReadOnly = readOnly;
    repaint();
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


FloatWatcher::FloatWatcher(ValueNode<NodeType::FLOAT>* node, WatcherWidget* widget, 
                           QString name)
  : Watcher(node, widget)
  , mIsReadOnly(false)
{
  float value = node->Get();

  QHBoxLayout* layout = new QHBoxLayout(widget);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);

  mSlider = new Slider(widget, name, node->Get(), 0, 1);
  mSlider->setMinimumHeight(20);
  mSlider->setMinimumWidth(70);
  mSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  mSlider->Set(value);
  layout->addWidget(mSlider);

  mTextBox = new TextBox(widget);
  SetTextBoxValue(value);
  mTextBox->setFixedWidth(40);
  layout->addWidget(mTextBox);

  mSlider->onValueChange += Delegate(this, &FloatWatcher::SliderValueChanged);
  mTextBox->onEditingFinished += Delegate(this, &FloatWatcher::SpinBoxValueChanged);

  mAllowTextboxValueChanges = true;
}

void FloatWatcher::SpinBoxValueChanged() {
  mAllowTextboxValueChanges = false;
  float value = mTextBox->text().toFloat();
  static_cast<FloatNode*>(GetNode())->Set(value);
  mAllowTextboxValueChanges = false;
}

void FloatWatcher::SetTextBoxValue(float value) {
  mTextBox->setText(QString::number(value, 'g', 3));
}

void FloatWatcher::SliderValueChanged(float value) {
  if (mIsReadOnly || !GetNode()) return;
  static_cast<FloatNode*>(GetNode())->Set(value);
}

void FloatWatcher::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
  switch (message) {
    case NodeMessage::VALUE_CHANGED: {
      float value = static_cast<FloatSlot*>(slot)->Get();
      mSlider->Set(value);
      if (mAllowTextboxValueChanges) SetTextBoxValue(value);
      break;
    }
    default: break;
  }
}

void FloatWatcher::SetReadOnly(bool readOnly) {
  if (mIsReadOnly != readOnly) {
    mIsReadOnly = readOnly;
    mTextBox->setEnabled(!readOnly);
    mSlider->SetReadOnly(readOnly);
  }
}

void FloatWatcher::HandleChangedNode(Node* node) {
  float value = node ? static_cast<ValueNode<NodeType::FLOAT>*>(node)->Get() : 0.0f;
  mSlider->Set(value);
  SetTextBoxValue(value);
}
