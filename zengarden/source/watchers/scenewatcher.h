#pragma once

#include "generalscenewatcher.h"

class SceneWatcher: public GeneralSceneWatcher {
public:
  SceneWatcher(const shared_ptr<Node>& scene);
};