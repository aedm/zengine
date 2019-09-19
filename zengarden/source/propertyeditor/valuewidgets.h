#pragma once

#include <zengine.h>
#include "../watchers/watcherui.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/qlineedit.h>

class QSpinBox;
class QLineEdit;
class QSlider;

/// A widget that displays a float value as a semi-filled horizontal bar.
/// Upon clicking into the bar, the value changes.
class Slider : public QWidget {
  Q_OBJECT

public:
  Slider(QWidget* parent, QString text, float value, float minimum, float maximum);
  virtual ~Slider() = default;

  Event<float> onValueChange;

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);

  void Set(float value);
  void SetRange(float minimum, float maximum);

  float Get() const;

private:
  void paintEvent(QPaintEvent *e);
  void mousePressEvent(QMouseEvent * event) override;
  void mouseMoveEvent(QMouseEvent * event) override;

  void HandleMouse(QMouseEvent * event);

  QString mText;
  float mMinimum;
  float	mMaximum;
  float mValue;
  bool mIsReadOnly;
};


/// A simple one-line text editor with an Event instead of a fckn Qt signal.
class TextBox : public QLineEdit {
  Q_OBJECT

public:
  TextBox(QWidget* parent);

  Event<> onEditingFinished;

  private slots:
  void HandleEditingFinished();
};

/// General template widget for editing float or vector values
template<ValueType T> class ValueEditor;


/// A widget able to edit and display a float value
template<> 
class ValueEditor<ValueType::FLOAT>: public QWidget {
public:
  //static constexpr ValueType Type = ValueType::FLOAT;

  ValueEditor(QWidget* parent, QString name, float value);

  /// Set the value of the editor
  void Set(float value);
  void SetRange(float minimum, float maximum) const;

  /// Subscribe to get value updates
  Event<QWidget*, const float&> onValueChange;

  /// Set read only mode
  void SetReadOnly(bool readOnly);

protected:
  void SetTextBoxValue(float value) const;
  void SliderValueChanged(float value);
  void SpinBoxValueChanged();

  /// Current value of the editor
  float mValue;

  /// Allow editing
  bool mIsReadOnly = false;

  TextBox* mTextBox = nullptr;
  Slider* mSlider = nullptr;

  /// Do not attempt to modify textbox when the new value is originated from there.
  bool mAllowTextboxValueChanges = true;
};


/// A widget able to edit and display a vector value
template <ValueType T>
class ValueEditor: public QWidget {
public:
  //static constexpr ValueType VType = VectorType;
  //typename ValueTypes<T>::Type;
  typedef typename ValueTypes<T>::Type VectorType;

  ValueEditor(QWidget* parent, QString name, const VectorType& value);

  /// Set the value of the editor
  void Set(const VectorType& value);
  void SetRange(float minimum, float maximum);

  /// Subscribe to get value updates
  Event<QWidget*, const VectorType&> onValueChange;

  /// Set read only mode
  void SetReadOnly(bool readOnly);

protected:
  typedef ValueEditor<ValueType::FLOAT> FloatEditor;

  /// Value of the vector
  VectorType mValue;

  /// Float editors for each dimension
  FloatEditor* mFloatEditors[VectorType::Dimensions];

  /// Handles value change from float editors
  void HandleFloatValueChange(QWidget* floatEditor, const float& value);
};
