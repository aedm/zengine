#pragma once

#include "generalscenewatcher.h"

class DrawableWatcher: public GeneralSceneWatcher {
public:
  DrawableWatcher(Drawable* drawable, GLWatcherWidget* watcherWidget);

};