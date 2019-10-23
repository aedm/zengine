#pragma once

#include "pass.h"
#include "../resources/mesh.h"
#include "../render/rendertarget.h"

class EngineShaders {
public:
  EngineShaders();
  ~EngineShaders();

  void ApplyPostProcess(RenderTarget* renderTarget, Globals* globals);
   
  const std::shared_ptr<Pass> mSolidShadowPass;

private:
  void BuildPostProcessPasses();
  void BuildMaterialPasses() const;

  /// Copies resolved color to a postprocess buffer
  /// in: Gbuffer (MSAA)
  /// out: postprocess source buffer
  static void BlitGBufferToPostprocessBuffers(RenderTarget* renderTarget, 
    Globals* globals);

  /// Generates depth of field effect
  /// in: Gbuffer (MSAA)
  /// out: DOF buffer (MSAA) + postprocess source buffer (no MSAA)
  void ApplyDepthOfField(RenderTarget* renderTarget, Globals* globals) const;

  /// Shrinks postprocess texture and applies Gaussian blur to it
  /// in: postprocess source buffer
  /// out: postprocess source buffer
  void GenerateBloomTexture(RenderTarget* renderTarget, Globals* globals);

  /// Clamps MSAA pixels and adds bloom to them
  /// in: Gbuffer (MSAA) + postprocess source buffer
  /// out: screen
  void RenderFinalImage(RenderTarget* renderTarget, Globals* globals, 
    const std::shared_ptr<Texture>& sourceColorMsaa) const;

  std::shared_ptr<Pass> mPostProcess_GaussianBlurHorizontal_First = std::make_shared<Pass>();
  std::shared_ptr<Pass> mPostProcess_GaussianBlurHorizontal = std::make_shared<Pass>();
  std::shared_ptr<Pass> mPostProcess_GaussianBlurVertical = std::make_shared<Pass>();
  std::shared_ptr<Pass> mPostProcess_GaussianBlur_Blend_MSAA = std::make_shared<Pass>();
  std::shared_ptr<Pass> mPostProcess_DOF = std::make_shared<Pass>();

public:
  std::shared_ptr<Pass> mFluid_CurlPass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_VorticityPass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_DivergencePass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_FadeOutPass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_PressurePass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_GradientSubtractPass = std::make_shared<Pass>();
  std::shared_ptr<Pass> mFluid_AdvectionPass = std::make_shared<Pass>();
    
  std::shared_ptr<Mesh> mFullScreenQuad;
};

extern EngineShaders* TheEngineShaders;