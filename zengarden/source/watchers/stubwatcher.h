#pragma once

#include "watcherui.h"
#include <zengine.h>

class StubWatcher : public WatcherUi
{
public:
	StubWatcher(const shared_ptr<StubNode>&);
	virtual ~StubWatcher();

};