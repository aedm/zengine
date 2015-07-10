#include "stubwatcher.h"

StubWatcher::StubWatcher(StubNode* Stub, WatcherWidget* Widget)
	: Watcher(Stub, Widget)
{
	ASSERT(Widget != NULL);

}

StubWatcher::~StubWatcher()
{

}
