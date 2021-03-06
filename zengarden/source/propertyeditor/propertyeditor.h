#pragma once

#include <zengine.h>
#include "../watchers/watcherui.h"
#include "valuewidgets.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

/// General node editor, displays name and type of the Node
class PropertyEditor: public WatcherUi {
public:
  PropertyEditor(const std::shared_ptr<Node>& node);
  virtual ~PropertyEditor() = default;

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void HandleNameTexBoxChanged() const;

  QBoxLayout*	mLayout = nullptr;
  TextBox* mNameTextBox = nullptr;
};


/// Editor for static float/vector nodes
template<ValueType T>
class StaticValueWatcher : public PropertyEditor {
public:
  typedef typename ValueTypes<T>::Type VectorType;
  StaticValueWatcher(const std::shared_ptr<StaticValueNode<VectorType>>& node);

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  ValueEditor<T>* mVectorEditor = nullptr;
  void HandleEditorValueChange(QWidget* editor, const VectorType& value);
};

