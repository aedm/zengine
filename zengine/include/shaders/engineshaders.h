#pragma once

#include "pass.h"
#include "../resources/mesh.h"
#include "../render/rendertarget.h"

class EngineShaders {
public:
  EngineShaders();
  ~EngineShaders();

  void ApplyPostProcess(RenderTarget* renderTarget, Globals* globals);
   
  const shared_ptr<Pass> mSolidShadowPass;

private:
  void BuildPostProcessPasses();
  void BuildMaterialPasses();

  /// Copies resolved color to a postprocess buffer
  /// in: Gbuffer (MSAA)
  /// out: postprocess source buffer
  void BlitGBufferToPostprocessBuffers(RenderTarget* renderTarget, Globals* globals);

  /// Generates depth of field effect
  /// in: Gbuffer (MSAA)
  /// out: DOF buffer (MSAA) + postprocess source buffer (no MSAA)
  void ApplyDepthOfField(RenderTarget* renderTarget, Globals* globals);

  /// Shrinks postprocess texture and applies Gaussian blur to it
  /// in: postprocess source buffer
  /// out: postprocess source buffer
  void GenerateBloomTexture(RenderTarget* renderTarget, Globals* globals);

  /// Clamps MSAA pixels and adds bloom to them
  /// in: Gbuffer (MSAA) + postprocess source buffer
  /// out: screen
  void RenderFinalImage(RenderTarget* renderTarget, Globals* globals, 
                         Texture* sourceColorMSAA);

  shared_ptr<Pass> mPostProcess_GaussianBlurHorizontal_First = make_shared<Pass>();
  shared_ptr<Pass> mPostProcess_GaussianBlurHorizontal = make_shared<Pass>();
  shared_ptr<Pass> mPostProcess_GaussianBlurVertical = make_shared<Pass>();
  shared_ptr<Pass> mPostProcess_GaussianBlur_Blend_MSAA = make_shared<Pass>();
  shared_ptr<Pass> mPostProcess_DOF = make_shared<Pass>();

  Mesh* mFullScreenQuad = nullptr;
};

extern EngineShaders* TheEngineShaders;