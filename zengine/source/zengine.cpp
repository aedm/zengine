#include <include/base/system.h>
#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>
#include <include/serialize/imageloader.h>

OpenGLAPI* OpenGL = nullptr;
EngineStubs* TheEngineStubs = nullptr;
EngineShaders* TheEngineShaders = nullptr;

bool PleaseNoNewResources = false;

Event<> OnZengineInitDone;

/// Initializes Zengine. Returns true if everything went okay.
bool InitZengine() {
  OpenGL = new OpenGLAPI();
  Zengine::InitGDIPlus();
  TheEngineStubs = new EngineStubs();
  OnZengineInitDone();
  return true;
}

/// Closes Zengine, frees up resources
void CloseZengine() {
  SafeDelete(TheEngineShaders);
  SafeDelete(TheEngineStubs);
  SafeDelete(OpenGL);

  /// Resources will be dropped with no GL context
  GLDisableErrorChecks = true;
}
