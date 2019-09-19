#pragma once

/// This file stores shader stubs required by the engine.

#include "stubnode.h"
#include <map>

using namespace std;


class EngineStubs {
public: 
  EngineStubs() = default;
  ~EngineStubs();

  /// Sets the stub source (normally tool does this)
  void SetStubSource(const string& name, const string& source);

  /// Get stub source by name
  shared_ptr<StubNode> GetStub(const string& name);
  string GetSource(const string& name);

  /// Engine stubs loaded, create built-in material passes and shaders
  void OnLoadFinished();

private:
  map<string, shared_ptr<StubNode>> mStubs;
};

extern EngineStubs* TheEngineStubs;
