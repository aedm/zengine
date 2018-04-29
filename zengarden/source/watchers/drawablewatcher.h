#pragma once

#include "generalscenewatcher.h"

class DrawableWatcher: public GeneralSceneWatcher {
public:
  DrawableWatcher(const shared_ptr<Node>& drawable);
};