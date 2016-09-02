#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>

EngineStubs::EngineStubs() {

}

EngineStubs::~EngineStubs() {
  for (auto &it : mStubs) delete it.second;
  mStubs.clear();
}

void EngineStubs::SetStubSource(const string& name, const string& source) {
  StubNode* stub;
  auto it = mStubs.find(name);
  if (it == mStubs.end()) {
    stub = new StubNode();
    mStubs[name] = stub;
  } else {
    stub = it->second;
  }
  stub->mSource.SetDefaultValue(source);
}

StubNode* EngineStubs::GetStub(const string& name) {
  return mStubs.at(name);
}

void EngineStubs::OnLoadFinished() {
  TheEngineShaders = new EngineShaders();
}
