#include "drawablewatcher.h"

DrawableWatcher::DrawableWatcher(const shared_ptr<Node>& drawable)
  : GeneralSceneWatcher(drawable)
{
  mTheScene->mDrawables.Connect(drawable);
}

