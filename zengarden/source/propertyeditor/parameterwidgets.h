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


/// A widget able to edit and display a float value
class FloatEditor: public QWidget {
public: 
  FloatEditor(QWidget* parent, QString name, float value);

  /// Set the value of the editor
  void Set(float value);

  /// Subscribe to get value updates
  Event<FloatEditor*, float> onValueChange;

  /// Set read only mode
  void SetReadOnly(bool readOnly);
  
protected:
  void SetTextBoxValue(float value);
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


/// A parameter panel item for FloatNodes
class FloatWatcher: public Watcher {
public:
  FloatWatcher(ValueNode<NodeType::FLOAT>* node, WatcherWidget* widget, QString name,
               bool readOnly);
  virtual ~FloatWatcher() {}

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);

protected:
  /// This method will be called when the watched node was changed
  virtual void HandleChangedNode(Node* node) override;

private:
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot, 
                                    void* payload) override;

  FloatEditor* mEditorX = nullptr;

  void HandleValueChange(FloatEditor* editor, float value);
};


/// A parameter panel item for Vec3Nodes
class Vec3Watcher: public Watcher {
public:
  Vec3Watcher(ValueNode<NodeType::VEC3>* node, WatcherWidget* widget, QString name,
              bool readOnly);
  virtual ~Vec3Watcher() {}

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);

protected:
  /// This method will be called when the watched node was changed
  virtual void HandleChangedNode(Node* node) override;

private:
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  FloatEditor* mEditorX = nullptr;
  FloatEditor* mEditorY = nullptr;
  FloatEditor* mEditorZ = nullptr;

  void HandleValueChange(FloatEditor* editor, float value);
};


/// A parameter panel item for Vec4Nodes
class Vec4Watcher: public Watcher {
public:
  Vec4Watcher(ValueNode<NodeType::VEC4>* node, WatcherWidget* widget, QString name,
              bool readOnly);
  virtual ~Vec4Watcher() {}

  /// Enable/disable editing
  void SetReadOnly(bool readOnly);

protected:
  /// This method will be called when the watched node was changed
  virtual void HandleChangedNode(Node* node) override;

private:
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  FloatEditor* mEditorX = nullptr;
  FloatEditor* mEditorY = nullptr;
  FloatEditor* mEditorZ = nullptr;
  FloatEditor* mEditorW = nullptr;

  void HandleValueChange(FloatEditor* editor, float value);
};
