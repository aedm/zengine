#include "drawablewatcher.h"

DrawableWatcher::DrawableWatcher(const std::shared_ptr<Node>& drawable)
  : GeneralSceneWatcher(drawable)
{
  mTheScene->mDrawables.Connect(drawable);
}

