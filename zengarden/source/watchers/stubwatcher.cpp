#include "stubwatcher.h"

StubWatcher::StubWatcher(StubNode* Stub, WatcherWidget* Widget)
	: WatcherUI(Stub, Widget)
{
	ASSERT(Widget != NULL);

}

StubWatcher::~StubWatcher()
{

}
