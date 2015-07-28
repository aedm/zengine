#pragma once

#include <zengine.h>
#include "../watchers/watcher.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <map>

class TextBox;

/// General node editor, displays name and type of the Node
class PropertyEditor: public Watcher {
public:
  PropertyEditor(Node* node, WatcherWidget* panel);
  virtual ~PropertyEditor() {}

protected:
  void HandleNameTexBoxChanged();

  QBoxLayout*	mLayout;
  TextBox* mNameTextBox;
};


/// Widget that displays node parameters
class DefaultPropertyEditor: public PropertyEditor {
public:
  DefaultPropertyEditor(Node* node, WatcherWidget* watcher);
  virtual ~DefaultPropertyEditor() {}

private:
  map<Slot*, Watcher*> mSlotWatchers;

  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  void RemoveWatcherWidget(WatcherWidget* watcherWidget);

};


/// Editor for static float nodes
class StaticFloatEditor: public PropertyEditor {
public:
  StaticFloatEditor(FloatNode* node, WatcherWidget* panel);
  virtual ~StaticFloatEditor() {}

private:
  WatcherWidget* mFloatWatcherWidget = nullptr;
  void RemoveFloatWatcher(WatcherWidget* watcherWidget);
};
