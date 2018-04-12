#include "drawablewatcher.h"

DrawableWatcher::DrawableWatcher(const shared_ptr<Drawable>& drawable)
  : GeneralSceneWatcher(drawable)
{
  mDefaultScene->mDrawables.Connect(drawable);
}

