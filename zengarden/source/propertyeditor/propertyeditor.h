#pragma once

#include <zengine.h>
#include "../watchers/watcherui.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <map>

class TextBox;

/// General node editor, displays name and type of the Node
class PropertyEditor: public WatcherUI {
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
  map<Slot*, WatcherUI*> mSlotWatchers;

  virtual void OnSlotConnectionChanged(Slot* slot) override;

  void RemoveWatcherWidget(WatcherWidget* watcherWidget);

};


/// Editor for static float nodes
class StaticFloatEditor: public PropertyEditor {
public:
  StaticFloatEditor(FloatNode* node, WatcherWidget* panel);
  virtual ~StaticFloatEditor() {}

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};


/// Editor for static vec3 nodes
class StaticVec3Editor: public PropertyEditor {
public:
  StaticVec3Editor(Vec3Node* node, WatcherWidget* panel);
  virtual ~StaticVec3Editor() {}

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};


/// Editor for static vec4 nodes
class StaticVec4Editor: public PropertyEditor {
public:
  StaticVec4Editor(Vec4Node* node, WatcherWidget* panel);
  virtual ~StaticVec4Editor() {}

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};
