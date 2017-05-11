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
  PropertyEditor(Node* node);
  virtual ~PropertyEditor() {}

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void HandleNameTexBoxChanged();

  QBoxLayout*	mLayout = nullptr;
  TextBox* mNameTextBox = nullptr;
};


/// Widget that displays node parameters
class DefaultPropertyEditor: public PropertyEditor {
public:
  DefaultPropertyEditor(Node* node);
  virtual ~DefaultPropertyEditor();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  map<Slot*, shared_ptr<WatcherUI>> mSlotWatchers;

  virtual void OnSlotConnectionChanged(Slot* slot) override;

  void RemoveWatcherWidget(WatcherWidget* watcherWidget);

};


/// Editor for static float nodes
class StaticFloatEditor: public PropertyEditor {
public:
  StaticFloatEditor(FloatNode* node);
  virtual ~StaticFloatEditor() {}

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};


/// Editor for static vec3 nodes
class StaticVec3Editor: public PropertyEditor {
public:
  StaticVec3Editor(Vec3Node* node);
  virtual ~StaticVec3Editor() {}

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};


/// Editor for static vec4 nodes
class StaticVec4Editor: public PropertyEditor {
public:
  StaticVec4Editor(Vec4Node* node);
  virtual ~StaticVec4Editor() {}

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  WatcherWidget* mValueWatcherWidget = nullptr;
  void RemoveStaticWatcher(WatcherWidget* watcherWidget);
};
