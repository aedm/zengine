#pragma once

#include "generalscenewatcher.h"

class SceneWatcher: public GeneralSceneWatcher {
public:
  SceneWatcher(SceneNode* scene);
  virtual ~SceneWatcher();
};