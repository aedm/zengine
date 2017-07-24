#include <include/resources/resourcemanager.h>
#include <include/base/system.h>
#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>

ResourceManager* TheResourceManager = nullptr;
OpenGLAPI* OpenGL = nullptr;
EngineStubs* TheEngineStubs = nullptr;
EngineShaders* TheEngineShaders = nullptr;

Event<> OnZengineInitDone;

/// Initializes Zengine. Returns true if everything went okay.
bool InitZengine() {
  OpenGL = new OpenGLAPI();
  TheResourceManager = new ResourceManager();
  TheEngineStubs = new EngineStubs();
  OnZengineInitDone();
  return true;
}

/// Closes Zengine, frees up resources
void CloseZengine() {
  SafeDelete(TheEngineShaders);
  SafeDelete(TheEngineStubs);
  SafeDelete(TheResourceManager);
  SafeDelete(OpenGL);
}
