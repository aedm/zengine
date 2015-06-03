#include "stubwatcher.h"

StubWatcher::StubWatcher(ShaderStub* Stub, WatcherWidget* Widget)
	: Watcher(Stub, Widget)
{
	ASSERT(Widget != NULL);

}

StubWatcher::~StubWatcher()
{

}
