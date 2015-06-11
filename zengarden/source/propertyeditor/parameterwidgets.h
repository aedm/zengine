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

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);

  void Set(float value);
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
  bool mIsReadOnly;
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
  FloatWatcher(ValueNode<NodeType::FLOAT>* node, WatcherWidget* widget, QString name);
  virtual ~FloatWatcher() {}

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);


protected:
  /// This method will be called when the watched node was changed
  virtual void HandleChangedNode(Node* node) override;


private:
  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  /// Allow editing
  bool mIsReadOnly;

  TextBox* mTextBox;
  Slider* mSlider;

  /// Do not attempt to modify textbox when the new value is originated from there.
  bool mAllowTextboxValueChanges;

  void SetTextBoxValue(float value);
  void SliderValueChanged(float Vlaue);
  void SpinBoxValueChanged();
};
