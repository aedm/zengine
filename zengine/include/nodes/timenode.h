#pragma once

#include "valuenodes.h"

/// This node 
class TimeNode: public FloatNode {
public:
  TimeNode();
  virtual ~TimeNode();

  static Event<float> OnTimeChanged;

private:
  void HandleTimeChange(float milliseconds);

};