#pragma once

#include "watcher.h"
#include <zengine.h>

class StubWatcher : public Watcher
{
public:
	StubWatcher(StubNode* Stub, WatcherWidget* Widget);
	virtual ~StubWatcher();

};