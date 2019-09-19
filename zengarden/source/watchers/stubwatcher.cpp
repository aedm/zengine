#include "stubwatcher.h"

StubWatcher::StubWatcher(const shared_ptr<StubNode>& Stub)
  : WatcherUI(Stub) 
{}

StubWatcher::~StubWatcher()
= default;
