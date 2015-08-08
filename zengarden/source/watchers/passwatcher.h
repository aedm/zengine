#pragma once

#include "generalscenewatcher.h"

class PassWatcher : public GeneralSceneWatcher
{
public:
	PassWatcher(Pass* pass, GLWatcherWidget* watcherWidget);
	virtual ~PassWatcher();

protected:
	Material mMaterial;
	MeshNode* mMesh;
  Drawable mDrawable;
};