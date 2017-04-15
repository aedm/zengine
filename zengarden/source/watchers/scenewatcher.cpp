#include "scenewatcher.h"


SceneWatcher::SceneWatcher(SceneNode* scene) 
  : GeneralSceneWatcher(scene)
{
  mScene = scene;
}

SceneWatcher::~SceneWatcher() {
}
