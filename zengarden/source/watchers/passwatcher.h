#pragma once

#include "generalscenewatcher.h"

class PassWatcher : public GeneralSceneWatcher
{
public:
  PassWatcher(const shared_ptr<Node>& pass);

protected:
  shared_ptr<Material> mMaterial = make_shared<Material>();
  shared_ptr<Drawable> mDrawable = make_shared<Drawable>();
  shared_ptr<MeshNode> mMesh;
};