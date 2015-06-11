#pragma once

#include <zengine.h>
#include "../watchers/watcher.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/qlineedit.h>

class QSpinBox;
class QLineEdit;
class QSlider;

/// A widget that displays a float value as a semi-filled horizontal bar.
/// Upon clicking into the bar, the value changes.
class Slider: public QWidget {
  Q_OBJECT

public:
  Slider(QWidget* parent, QString text, float value, float minimum, float maximum);
  virtual ~Slider() {}

  Event<float> onValueChange;

  void Set(float Value);
  float Get();

private:
  void paintEvent(QPaintEvent *e);
  virtual void mousePressEvent(QMouseEvent * event) override;
  virtual void mouseMoveEvent(QMouseEvent * event) override;

  void HandleMouse(QMouseEvent * event);

  QString mText;
  float mMinimum;
  float	mMaximum;
  float mValue;
};


/// A simple one-line text editor with an Event instead of a fckn Qt signal.
class TextBox: public QLineEdit {
  Q_OBJECT

public:
  TextBox(QWidget* parent);

  Event<> onEditingFinished;

  private slots:
  void HandleEditingFinished();
};


/// A parameter panel item for FloatNodes
class FloatWatcher: public Watcher {
public:
  FloatWatcher(FloatNode* node, WatcherWidget* widget);
  virtual ~FloatWatcher() {}

private:
  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  TextBox* mTextBox;
  Slider* mSlider;

  /// Do not attempt to modify textbox when the new value is originated from there.
  bool mAllowTextboxValueChanges;

  void SetTextBoxValue(float value);
  void SliderValueChanged(float Vlaue);
  void SpinBoxValueChanged();
};
