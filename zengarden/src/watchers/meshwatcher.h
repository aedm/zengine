#pragma once

#include "generalscenewatcher.h"
#include <memory>

class MeshWatcher : public GeneralSceneWatcher {
public:
  MeshWatcher(const std::shared_ptr<Node>& meshNode);

private:
  std::shared_ptr<Drawable> mDrawable = std::make_shared<Drawable>();
};
