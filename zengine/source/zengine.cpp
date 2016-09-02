#include "render/opengl/drawingopengl.h"
#include <include/resources/resourcemanager.h>
#include <include/base/system.h>
#include <include/shaders/enginestubs.h>
#include <include/shaders/engineshaders.h>

ResourceManager* TheResourceManager = nullptr;
DrawingAPI* TheDrawingAPI = nullptr;
EngineStubs* TheEngineStubs = nullptr;
EngineShaders* TheEngineShaders = nullptr;

Event<> OnZengineInitDone;

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine() {
  TheDrawingAPI = new DrawingOpenGL();
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
  SafeDelete(TheDrawingAPI);
}
