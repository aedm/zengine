#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>
#include <memory>

EngineStubs::~EngineStubs() {
  for (auto& stub : mStubs) {
    stub.second->Dispose();
  }
}

void EngineStubs::SetStubSource(const std::string& name, const std::string& source) {
  std::shared_ptr<StubNode> stub;
  const auto& it = mStubs.find(name);
  if (it == mStubs.end()) {
    stub = std::make_shared<StubNode>();
    mStubs[name] = stub;
  }
  else {
    stub = it->second;
  }
  stub->mSource.SetDefaultValue(source);
}

std::shared_ptr<StubNode> EngineStubs::GetStub(const std::string& name) {
  return mStubs.at(name);
}

std::string EngineStubs::GetSource(const std::string& name) {
  return mStubs.at(name)->mSource.GetDefaultValue();
}

void EngineStubs::OnLoadFinished() {
  TheEngineShaders = new EngineShaders();
}
