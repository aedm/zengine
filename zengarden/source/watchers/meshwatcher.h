#pragma once

#include "generalscenewatcher.h"

class MeshWatcher : public GeneralSceneWatcher {
public:
  MeshWatcher(const shared_ptr<Node>& meshNode);

private:
  shared_ptr<Drawable> mDrawable = make_shared<Drawable>();
};