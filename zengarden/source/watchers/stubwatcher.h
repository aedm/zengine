#pragma once

#include "watcher.h"
#include <zengine.h>

class StubWatcher : public Watcher
{
public:
	StubWatcher(ShaderStub* Stub, WatcherWidget* Widget);
	virtual ~StubWatcher();

};