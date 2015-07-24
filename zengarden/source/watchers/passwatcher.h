#pragma once

#include "watcher.h"
#include "watcherwidget.h"
#include <zengine.h>

class PassWatcher : public Watcher
{
public:
	PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget);
	virtual ~PassWatcher();

protected:
	void Paint(GLWidget* widget);
	virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, 
                                    void* payload) override;

	Drawable* mDrawable;
	Material* mMaterial;
	MeshNode* mMesh;
	Globals mGlobals;
};