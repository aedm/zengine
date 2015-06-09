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
	void			Paint(GLWidget* Widget);
	virtual void	HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload) override;

	Drawable*		TheDrawable;
	Material*		TheMaterial;
	MeshNode*		TheMesh;
	Globals			TheGlobals;
};