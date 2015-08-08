#include "meshwatcher.h"

MeshWatcher::MeshWatcher(MeshNode* meshNode, GLWatcherWidget* watcherWidget)
  : GeneralSceneWatcher(meshNode, watcherWidget)
{
  mDrawable.mMaterial.Connect(mDefaultMaterial);
  mDrawable.mMesh.Connect(meshNode);
  mDefaultScene.mDrawables.Connect(&mDrawable);
}
