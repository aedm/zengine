#pragma once

#include "watcherui.h"
#include <zengine.h>

class StubWatcher : public WatcherUI
{
public:
	StubWatcher(StubNode* Stub, WatcherWidget* Widget);
	virtual ~StubWatcher();

};