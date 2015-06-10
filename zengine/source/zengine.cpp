#include "render/opengl/drawingopengl.h"
#include <include/resources/resourcemanager.h>
#include <include/base/system.h>

ResourceManager* TheResourceManager = NULL;
DrawingAPI* TheDrawingAPI = NULL;

Event<> OnZengineInitDone;

/// Inits Zengine. Returns true if everything went okay.
bool InitZengine() {
  TheDrawingAPI = new DrawingOpenGL();
  TheResourceManager = new ResourceManager();
  OnZengineInitDone();
  return true;
}

/// Closes Zengine, frees up resources
void CloseZengine() {
  SafeDelete(TheResourceManager);
  SafeDelete(TheDrawingAPI);
}
