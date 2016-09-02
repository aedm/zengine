#pragma once

/// This file stores shader stubs required by the engine.

#include "stubnode.h"
#include <map>

using namespace std;


class EngineStubs {
public: 
  EngineStubs();
  ~EngineStubs();

  /// Sets the stub source (normally tool does this)
  void SetStubSource(const string& name, const string& source);

  /// Get stub source by name
  StubNode* GetStub(const string& name);

  /// Engine stutbs loaded, create built-in material passes and shaders
  void OnLoadFinished();

private:
  map<string, StubNode*> mStubs;
};

extern EngineStubs* TheEngineStubs;
