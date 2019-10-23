#pragma once

/// This file stores shader stubs required by the engine.

#include "stubnode.h"
#include <map>

class EngineStubs {
public: 
  EngineStubs() = default;
  ~EngineStubs();

  /// Sets the stub source (normally tool does this)
  void SetStubSource(const std::string& name, const std::string& source);

  /// Get stub source by name
  std::shared_ptr<StubNode> GetStub(const std::string& name);
  std::string GetSource(const std::string& name);

  /// Engine stubs loaded, create built-in material passes and shaders
  static void OnLoadFinished();

private:
  std::map<std::string, std::shared_ptr<StubNode>> mStubs;
};

extern EngineStubs* TheEngineStubs;
