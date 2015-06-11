#pragma once

#include <zengine.h>
#include "../watchers/watcher.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <map>

/// General node editor, displays name and type of the Node
class PropertyEditor: public Watcher {
public:
  PropertyEditor(Node* node, WatcherWidget* panel);
  virtual ~PropertyEditor() {}

protected:
  QBoxLayout*	mLayout;
};


/// Widget that displays node parameters
class DefaultPropertyEditor: public PropertyEditor {
public:
  DefaultPropertyEditor(Node* node, WatcherWidget* watcher);
  virtual ~DefaultPropertyEditor() {}

private:
  map<Slot*, Watcher*> mSlotWatchers;

  virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, const void* payload);
};


/// Editor for static float nodes
class StaticFloatEditor: public PropertyEditor {
public:
  StaticFloatEditor(FloatNode* node, WatcherWidget* panel);
  virtual ~StaticFloatEditor() {}
};