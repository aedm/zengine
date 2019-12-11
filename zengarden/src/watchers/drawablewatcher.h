#pragma once

#include "generalscenewatcher.h"

class DrawableWatcher: public GeneralSceneWatcher {
public:
  DrawableWatcher(const std::shared_ptr<Node>& drawable);
};