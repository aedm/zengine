#pragma once

#include "generalscenewatcher.h"
#include <memory>
#include <memory>
#include <memory>

class PassWatcher : public GeneralSceneWatcher
{
public:
  PassWatcher(const std::shared_ptr<Node>& pass);

protected:
  std::shared_ptr<Material> mMaterial = std::make_shared<Material>();
  std::shared_ptr<Drawable> mDrawable = std::make_shared<Drawable>();
  std::shared_ptr<StaticMeshNode> mMesh = std::make_shared<StaticMeshNode>();
};
