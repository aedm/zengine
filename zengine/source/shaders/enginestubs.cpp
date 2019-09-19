#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>

EngineStubs::~EngineStubs() {
  for (auto& stub : mStubs) {
    stub.second->Dispose();
  }
}

void EngineStubs::SetStubSource(const std::string& name, const std::string& source) {
  shared_ptr<StubNode> stub;
  auto& it = mStubs.find(name);
  if (it == mStubs.end()) {
    stub = make_shared<StubNode>();
    mStubs[name] = stub;
  }
  else {
    stub = it->second;
  }
  stub->mSource.SetDefaultValue(source);
}

shared_ptr<StubNode> EngineStubs::GetStub(const string& name) {
  return mStubs.at(name);
}

std::string EngineStubs::GetSource(const string& name) {
  return mStubs.at(name)->mSource.GetDefaultValue();
}

void EngineStubs::OnLoadFinished() {
  TheEngineShaders = new EngineShaders();
}
