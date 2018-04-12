#pragma once

#include "watcherui.h"
#include <zengine.h>

class StubWatcher : public WatcherUI
{
public:
	StubWatcher(const shared_ptr<StubNode>&);
	virtual ~StubWatcher();

};