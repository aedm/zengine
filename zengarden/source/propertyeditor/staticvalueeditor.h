#pragma once

#include "propertyeditor.h"

class StaticFloatEditor: public PropertyEditor {
public:
  StaticFloatEditor(FloatNode* node, WatcherWidget* panel);
  virtual ~StaticFloatEditor() {}
};