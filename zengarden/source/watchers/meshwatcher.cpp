#include "meshwatcher.h"

MeshWatcher::MeshWatcher(const shared_ptr<MeshNode>& meshNode)
  : GeneralSceneWatcher(meshNode)
{
  mDrawable->mMaterial.Connect(mDefaultMaterial);
  mDrawable->mMesh.Connect(meshNode);
  mDefaultScene->mDrawables.Connect(mDrawable);
}
