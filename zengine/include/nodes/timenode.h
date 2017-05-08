#pragma once

#include "valuenodes.h"


/// This node stores the global time
class GlobalTimeNode: public FloatNode {
public:
  GlobalTimeNode();
  virtual ~GlobalTimeNode();
  static Event<float> OnTimeChanged;

private:
  void HandleTimeChange(float milliseconds);
};

/// This node stores the scene time
class SceneTimeNode: public FloatNode {
public:
  SceneTimeNode();
};