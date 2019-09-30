#include "stubwatcher.h"

StubWatcher::StubWatcher(const shared_ptr<StubNode>& Stub)
  : WatcherUi(Stub) 
{}

StubWatcher::~StubWatcher()
= default;
