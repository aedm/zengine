#pragma once

#include "pass.h"
#include "../resources/mesh.h"
#include "../render/rendertarget.h"

class EngineShaders {
public:
  EngineShaders();
  ~EngineShaders();

  void ApplyPostProcess(RenderTarget* renderTarget, Globals* globals);
    
private:
  void BuildPostProcessPasses();

  Pass mPostProcess_GaussianBlurHorizontal_First;
  Pass mPostProcess_GaussianBlurHorizontal;
  Pass mPostProcess_GaussianBlurVertical;
  Pass mPostProcess_GaussianBlurVertical_Last;

  Mesh* mFullScreenQuad = nullptr;
};

extern EngineShaders* TheEngineShaders;