#pragma once

#include "valuenodes.h"


/// This node stores the global time
class GlobalTimeNode: public FloatNode {
public:
  GlobalTimeNode();
  virtual ~GlobalTimeNode();
  static Event<float> OnTimeChanged;

private:
  void HandleTimeChange(float beats);
};

/// This node stores the scene time
class SceneTimeNode: public FloatNode {
public:
  SceneTimeNode();

  /// Manually sets time. This function has the same effect as Set(), except
  /// it sends different signal indicating user input.
  void EditTime(float time);
};