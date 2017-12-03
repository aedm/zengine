#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>
#include <include/resources/resourcemanager.h>

#define GLEW_STATIC
#include <glew/glew.h>

static const UINT BloomEffectMaxResolution = 256;

EngineShaders::EngineShaders() {
  BuildPostProcessPasses();
  BuildMaterialPasses();
}

EngineShaders::~EngineShaders() {
  TheResourceManager->DiscardMesh(mFullScreenQuad);
}


void EngineShaders::ApplyPostProcess(RenderTarget* renderTarget, Globals* globals) {
  if (globals->PPDofEnabled < 0.5f) {
    BlitGBufferToPostprocessBuffers(renderTarget, globals);
    GenerateBloomTexture(renderTarget, globals);
    RenderFinalImage(renderTarget, globals, renderTarget->mGBufferA);
  } else {
    ApplyDepthOfField(renderTarget, globals);
    GenerateBloomTexture(renderTarget, globals);
    RenderFinalImage(renderTarget, globals, renderTarget->mDOFColorTexture);
  }
}


void EngineShaders::BlitGBufferToPostprocessBuffers(RenderTarget* renderTarget, 
                                                    Globals* globals) {
  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);

  /// Blit G-Buffer into postprocess ping-pong buffers 
  OpenGL->BlitFrameBuffer(renderTarget->mGBufferId,
                          renderTarget->GetPostprocessTargetFramebufferId(),
                          0, 0, width, height, 0, 0, width, height);
  renderTarget->SwapPostprocessBuffers();
}


void EngineShaders::ApplyDepthOfField(RenderTarget* renderTarget, Globals* globals) {
  mPostProcess_DOF.Update();
  if (!mPostProcess_DOF.isComplete()) return;
  
  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);

  OpenGL->SetFrameBuffer(renderTarget->mDOFBufferId);
  glViewport(0, 0, width, height);
  globals->GBufferSourceA = renderTarget->mGBufferA;
  globals->DepthBufferSource = renderTarget->mDepthBuffer;

  mPostProcess_DOF.Set(globals);
  mFullScreenQuad->Render(mPostProcess_DOF.GetUsedAttributes(), 1, PRIMITIVE_TRIANGLES);

  OpenGL->BlitFrameBuffer(renderTarget->mDOFBufferId,
                          renderTarget->GetPostprocessTargetFramebufferId(),
                          0, 0, width, height, 0, 0, width, height);
  renderTarget->SwapPostprocessBuffers();
}

void EngineShaders::GenerateBloomTexture(RenderTarget* renderTarget, Globals* globals) {
  mPostProcess_GaussianBlurHorizontal_First.Update();
  if (!mPostProcess_GaussianBlurHorizontal_First.isComplete()) return;
  mPostProcess_GaussianBlurHorizontal.Update();
  if (!mPostProcess_GaussianBlurHorizontal.isComplete()) return;
  mPostProcess_GaussianBlurVertical.Update();
  if (!mPostProcess_GaussianBlurVertical.isComplete()) return;

  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);
  UINT originalWidth = width;
  UINT originalHeight = height;

  UINT downsampleCount = UINT(ceilf(log2f(size.x / float(BloomEffectMaxResolution))));

  /// Decrease resolution
  for (UINT i = 0; i < downsampleCount; i++) {
    OpenGL->BlitFrameBuffer(renderTarget->GetPostprocessSourceFramebufferId(),
                            renderTarget->GetPostprocessTargetFramebufferId(),
                            0, 0, width, height, 0, 0, width / 2, height / 2);
    width /= 2;
    height /= 2;
    renderTarget->SwapPostprocessBuffers();
  }

  /// Blur the image
  size = Vec2(float(width), float(height));
  globals->PPGaussRelativeSize = Vec2(float(width) / float(originalWidth),
                                      float(height) / float(originalHeight));
  globals->PPGaussPixelSize = Vec2(1.0f, 1.0f) / renderTarget->GetSize();

  UINT gaussIterationCount = 1;

  for (UINT i = 0; i < gaussIterationCount * 2; i++) {
    OpenGL->SetFrameBuffer(renderTarget->GetPostprocessTargetFramebufferId());
    globals->PPGauss = renderTarget->GetPostprocessSourceTexture();
    Pass* pass = (i % 2 == 0)
      ? &mPostProcess_GaussianBlurHorizontal : &mPostProcess_GaussianBlurVertical;
    if (i == 0) {
      pass = &mPostProcess_GaussianBlurHorizontal_First;
    }
    if (i <= 1) {
      glViewport(0, 0, originalWidth, originalHeight);
      OpenGL->Clear(true, false, 0);
      glViewport(0, 0, width, height);
    }
    pass->Set(globals);
    mFullScreenQuad->Render(pass->GetUsedAttributes(), 1, PRIMITIVE_TRIANGLES);
    renderTarget->SwapPostprocessBuffers();
  }
}


void EngineShaders::RenderFinalImage(RenderTarget* renderTarget, Globals* globals,
                                      Texture* sourceColorMSAA) {
  mPostProcess_GaussianBlur_Blend_MSAA.Update();
  if (!mPostProcess_GaussianBlur_Blend_MSAA.isComplete()) return;

  /// Additively blend bloom to Gbuffer, and perform HDR multisampling correction
  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);
  OpenGL->SetFrameBuffer(renderTarget->mColorBufferId);
  glViewport(0, 0, width, height);
  globals->PPGauss = renderTarget->GetPostprocessSourceTexture();
  globals->GBufferSourceA = sourceColorMSAA;
  mPostProcess_GaussianBlur_Blend_MSAA.Set(globals);
  mFullScreenQuad->Render(mPostProcess_GaussianBlur_Blend_MSAA.GetUsedAttributes(), 1,
                          PRIMITIVE_TRIANGLES);
}

void EngineShaders::BuildPostProcessPasses() {
  StubNode* fullscreenVertex =
    TheEngineStubs->GetStub("postprocess/fullscreen-vertex");
  StubNode* gaussianHorizontalFirst =
    TheEngineStubs->GetStub("postprocess/gaussianblur-horizontal-first");
  StubNode* gaussianHorizontal =
    TheEngineStubs->GetStub("postprocess/gaussianblur-horizontal");
  StubNode* gaussianVertical =
    TheEngineStubs->GetStub("postprocess/gaussianblur-vertical");
  StubNode* gaussianBlendMSAA =
    TheEngineStubs->GetStub("postprocess/gaussianblur-blend-msaa");
  StubNode* dofFragment =
    TheEngineStubs->GetStub("postprocess/depth-of-field");


  mPostProcess_GaussianBlurHorizontal.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal.mFragmentStub.Connect(gaussianHorizontal);
  mPostProcess_GaussianBlurHorizontal.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurHorizontal.mBlendModeSlot.SetDefaultValue(1.0f); // normal
  mPostProcess_GaussianBlurHorizontal.mFaceModeSlot.SetDefaultValue(0.5f); // f&b
  mPostProcess_GaussianBlurHorizontal.Update();

  mPostProcess_GaussianBlurHorizontal_First.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal_First.mFragmentStub.Connect(
    gaussianHorizontalFirst);
  mPostProcess_GaussianBlurHorizontal_First.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurHorizontal_First.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlurHorizontal_First.mFaceModeSlot.SetDefaultValue(0.5f);
  mPostProcess_GaussianBlurHorizontal_First.Update();

  mPostProcess_GaussianBlurVertical.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurVertical.mFragmentStub.Connect(gaussianVertical);
  mPostProcess_GaussianBlurVertical.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurVertical.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlurVertical.mFaceModeSlot.SetDefaultValue(0.5f);
  mPostProcess_GaussianBlurVertical.Update();

  mPostProcess_GaussianBlur_Blend_MSAA.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlur_Blend_MSAA.mFragmentStub.Connect(gaussianBlendMSAA);
  mPostProcess_GaussianBlur_Blend_MSAA.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlur_Blend_MSAA.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlur_Blend_MSAA.mFaceModeSlot.SetDefaultValue(0.5f);
  mPostProcess_GaussianBlur_Blend_MSAA.Update();

  mPostProcess_DOF.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_DOF.mFragmentStub.Connect(dofFragment);
  mPostProcess_DOF.mRenderstate.mDepthTest = false;
  mPostProcess_DOF.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_DOF.mFaceModeSlot.SetDefaultValue(0.5f);
  mPostProcess_DOF.Update();

  /// Fullscreen quad
  IndexEntry quadIndices[] = {0, 1, 2, 2, 1, 3};
  VertexPosUV quadVertices[] = {
    {Vec3(-1, -1, 0), Vec2(0, 0)},
    {Vec3(1, -1, 0), Vec2(1, 0)},
    {Vec3(-1, 1, 0), Vec2(0, 1)},
    {Vec3(1, 1, 0), Vec2(1, 1)},
  };

  mFullScreenQuad = TheResourceManager->CreateMesh();
  mFullScreenQuad->SetIndices(quadIndices);
  mFullScreenQuad->SetVertices(quadVertices);
}

void EngineShaders::BuildMaterialPasses() {
  mSolidShadowPass.mVertexStub.Connect(
    TheEngineStubs->GetStub("material/solid/shadowPass-vertex"));
  mSolidShadowPass.mFragmentStub.Connect(
    TheEngineStubs->GetStub("material/solid/shadowPass-fragment"));

  mSolidShadowPass.mRenderstate.mDepthTest = true;
  mSolidShadowPass.mBlendModeSlot.SetDefaultValue(0.0f); // normal
  mSolidShadowPass.mFaceModeSlot.SetDefaultValue(1.0f); // back
}
