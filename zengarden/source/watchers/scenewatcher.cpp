#include "scenewatcher.h"


SceneWatcher::SceneWatcher(const shared_ptr<Node>& scene) 
  : GeneralSceneWatcher(scene)
{
  mScene = scene;
}

