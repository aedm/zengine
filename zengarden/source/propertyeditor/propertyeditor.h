#pragma once

#include <zengine.h>
#include "../watchers/watcherui.h"
#include "valuewidgets.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <map>

/// General node editor, displays name and type of the Node
class PropertyEditor: public WatcherUI {
public:
  PropertyEditor(Node* node);
  virtual ~PropertyEditor() {}

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void HandleNameTexBoxChanged();

  QBoxLayout*	mLayout = nullptr;
  TextBox* mNameTextBox = nullptr;
};


/// Editor for static float/vector nodes
template<ValueType T>
class StaticValueWatcher : public PropertyEditor {
public:
  typedef typename ValueTypes<T>::Type VectorType;
  StaticValueWatcher(StaticValueNode<T>* node);

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  ValueEditor<T>* mVectorEditor = nullptr;
  void HandleEditorValueChange(QWidget* editor, const VectorType& value);
};

