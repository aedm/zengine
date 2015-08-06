#include "meshwatcher.h"

MeshWatcher::MeshWatcher(MeshNode* meshNode, GLWatcherWidget* watcherWidget)
  : GeneralSceneWatcher(meshNode, watcherWidget)
{
  mDrawable = new Drawable();
  mDrawable->mMaterial.Connect(mDefaultMaterial);
  mDrawable->mMesh.Connect(meshNode);
}
