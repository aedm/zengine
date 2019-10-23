#include "stubwatcher.h"

StubWatcher::StubWatcher(const std::shared_ptr<StubNode>& Stub)
  : WatcherUi(Stub) 
{}

StubWatcher::~StubWatcher()
= default;
