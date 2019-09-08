#include "meshwatcher.h"

MeshWatcher::MeshWatcher(const shared_ptr<Node>& meshNode)
  : GeneralSceneWatcher(meshNode)
{
  mDrawable->mMaterial.Connect(mDefaultMaterial);
  mDrawable->mMesh.Connect(meshNode);
  mTheScene->mDrawables.Connect(mDrawable);
}
