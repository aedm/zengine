#pragma once

#include "generalscenewatcher.h"

class SceneWatcher: public GeneralSceneWatcher {
public:
  SceneWatcher(const std::shared_ptr<Node>& scene);
};