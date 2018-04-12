#include "scenewatcher.h"


SceneWatcher::SceneWatcher(const shared_ptr<SceneNode>& scene) 
  : GeneralSceneWatcher(scene)
{
  mScene = scene;
}

