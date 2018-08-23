#pragma once

#pragma once

#include "../dom/node.h"
#include "valuenodes.h"

class PropertiesNode: public Node {
public:
  PropertiesNode();

  FloatSlot mBPM;
};

typedef TypedSlot<PropertiesNode> PropertiesSlot;
