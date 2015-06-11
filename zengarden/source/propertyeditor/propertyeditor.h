#pragma once

#include <zengine.h>
#include "../watchers/watcher.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

/// General node editor, displays name and type of the Node
class PropertyEditor: public Watcher {
public:
  PropertyEditor(Node* node, WatcherWidget* panel);
  virtual ~PropertyEditor();

protected:
  QBoxLayout*	mLayout;
};


/// Widget that displays node parameters
