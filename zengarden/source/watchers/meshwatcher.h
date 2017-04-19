#pragma once

#include "generalscenewatcher.h"

class MeshWatcher : public GeneralSceneWatcher {
public:
  MeshWatcher(MeshNode* meshNode);

private:
  Drawable mDrawable;
};