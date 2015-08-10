#include "drawablewatcher.h"

DrawableWatcher::DrawableWatcher(Drawable* drawable, GLWatcherWidget* watcherWidget)
  : GeneralSceneWatcher(drawable, watcherWidget)
{
  mDefaultScene.mDrawables.Connect(drawable);
}

