#include "valuewidgets.h"
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
  , mIsReadOnly(false) {}

void Slider::paintEvent(QPaintEvent *e) {
  QPainter painter(this);
  const QSize widgetSize = size();
  painter.setPen(palette().dark().color().darker());
  painter.setBrush(palette().alternateBase().color());
  painter.drawRect(0, 0, widgetSize.width() - 1, widgetSize.height() - 1);

  if (mMaximum > mMinimum) {
    painter.setPen(Qt::NoPen);
    if (mIsReadOnly) painter.setBrush(palette().highlight().color().darker());
    else painter.setBrush(palette().highlight().color());

    const int widthPx = widgetSize.width() - 2;
    int full = float(widthPx) * (mValue - mMinimum) / (mMaximum - mMinimum);
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
    const float newValue = 
      mMinimum + float(event->x() + 1) * (mMaximum - mMinimum) / float(width());
    if (mValue != newValue) {
      onValueChange(newValue);
    }
  }
}

float Slider::Get() const
{
  return mValue;
}

void Slider::Set(float value) {
  if (value != mValue) {
    this->mValue = value;
    update();
  }
}

void Slider::SetRange(float minimum, float maximum) {
  mMinimum = minimum;
  mMaximum = maximum;
  update();
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

ValueEditor<ValueType::FLOAT>::ValueEditor(QWidget* parent, QString name, float value)
  : QWidget(parent)
  , mValue(value) 
{
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

  mSlider->onValueChange += 
    Delegate(this, &ValueEditor<ValueType::FLOAT>::SliderValueChanged);
  mTextBox->onEditingFinished += 
    Delegate(this, &ValueEditor<ValueType::FLOAT>::SpinBoxValueChanged);
}


void ValueEditor<ValueType::FLOAT>::SetTextBoxValue(float value) const
{
  if (mAllowTextboxValueChanges) {
    mTextBox->setText(QString::number(value, 'g', 3));
  }
}


void ValueEditor<ValueType::FLOAT>::SliderValueChanged(float value) {
  if (mIsReadOnly || value == mValue) return;
  mValue = value;
  SetTextBoxValue(value);
  /// TODO: slider should set and repaint itself
  mSlider->Set(value);
  onValueChange(this, value);
}


void ValueEditor<ValueType::FLOAT>::SpinBoxValueChanged() {
  const float value = mTextBox->text().toFloat();
  if (mIsReadOnly || value == mValue) return;
  /// TODO: slider should set and repaint itself
  mSlider->Set(value);
  mAllowTextboxValueChanges = false;
  onValueChange(this, value);
  mAllowTextboxValueChanges = true;
}


void ValueEditor<ValueType::FLOAT>::Set(float value) {
  if (mIsReadOnly || value == mValue) return;
  mValue = value;
  SetTextBoxValue(value);
  mSlider->Set(value);
}

void ValueEditor<ValueType::FLOAT>::SetRange(float minimum, float maximum) const
{
  mSlider->SetRange(minimum, maximum);
}

void ValueEditor<ValueType::FLOAT>::SetReadOnly(bool readOnly) {
  mSlider->SetReadOnly(readOnly);
  mTextBox->setReadOnly(readOnly);
  mIsReadOnly = readOnly;
}



template <ValueType T>
ValueEditor<T>::ValueEditor(QWidget* parent, QString name, const VectorType& value) {
  static const char* suffixes[] = { ".x", ".y", ".z", ".w" };
  mValue = value;
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);
  for (UINT i = 0; i < VectorType::Dimensions; i++) {
    FloatEditor* floatEditor = new FloatEditor(parent, name + suffixes[i], value[i]);
    floatEditor->onValueChange += Delegate(this, &ValueEditor<T>::HandleFloatValueChange);
    layout->addWidget(floatEditor);
    mFloatEditors[i] = floatEditor;
  }
}

template <ValueType T>
void ValueEditor<T>::Set(const VectorType& value) {
  mValue = value;
  for (UINT i = 0; i < VectorType::Dimensions; i++) {
    mFloatEditors[i]->Set(value[i]);
  }
}


template <ValueType T>
void ValueEditor<T>::SetRange(float minimum, float maximum) {
  for (UINT i = 0; i < VectorType::Dimensions; i++) {
    mFloatEditors[i]->SetRange(minimum, maximum);
  }
}


template <ValueType T>
void ValueEditor<T>::SetReadOnly(bool readOnly) {
  for (UINT i = 0; i < VectorType::Dimensions; i++) {
    mFloatEditors[i]->SetReadOnly(readOnly);
  }
}


template <ValueType T>
void ValueEditor<T>::HandleFloatValueChange(QWidget* floatEditor,
  const float& value) {
  for (UINT i = 0; i < VectorType::Dimensions; i++) {
    if (mFloatEditors[i] == floatEditor) {
      float& target = mValue[i];
      if (target != value) {
        target = value;
        onValueChange(this, mValue);
      }
      return;
    }
  }
}

template class ValueEditor<ValueType::FLOAT>;
template class ValueEditor<ValueType::VEC2>;
template class ValueEditor<ValueType::VEC3>;
template class ValueEditor<ValueType::VEC4>;