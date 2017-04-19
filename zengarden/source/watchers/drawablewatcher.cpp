#include "drawablewatcher.h"

DrawableWatcher::DrawableWatcher(Drawable* drawable)
  : GeneralSceneWatcher(drawable)
{
  mDefaultScene.mDrawables.Connect(drawable);
}

