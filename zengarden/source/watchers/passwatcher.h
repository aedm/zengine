#pragma once

#include "watcher.h"
#include "watcherwidget.h"
#include <zengine.h>

class PassWatcher : public Watcher
{
public:
	PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget);
	virtual ~PassWatcher();
};