#include "scenewatcher.h"


SceneWatcher::SceneWatcher(SceneNode* scene, GLWatcherWidget* watcherWidget) 
  : GeneralSceneWatcher(scene, watcherWidget)
{
  mScene = scene;
}

SceneWatcher::~SceneWatcher() {
}
