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
  mPostProcess_GaussianBlurHorizontal_First.Update();
  mPostProcess_GaussianBlurHorizontal.Update();
  mPostProcess_GaussianBlurVertical.Update();
  mPostProcess_GaussianBlur_Blend.Update();
  if (!mPostProcess_GaussianBlurHorizontal.isComplete()
      || !mPostProcess_GaussianBlurVertical.isComplete()
      || !mPostProcess_GaussianBlurHorizontal_First.isComplete()
      || !mPostProcess_GaussianBlur_Blend.isComplete()) return;

  UINT targetBufferIndex = 0;

  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);
  UINT originalWidth = width;
  UINT originalHeight = height;

  UINT downsampleCount = UINT(ceilf(log2f(size.x / float(BloomEffectMaxResolution))));

  /// Blit G-Buffer into gaussian ping-pong buffers to decrease resolution
  FrameBufferId source = renderTarget->mGBufferId;
  UINT newWidth = width;
  UINT newHeight = height;
  for (UINT i = 0; i <= downsampleCount; i++) {
    FrameBufferId target = renderTarget->mGaussFramebuffers[targetBufferIndex];
    OpenGL->BlitFrameBuffer(source, target, 0, 0, width, height, 0, 0, newWidth, newHeight);
    width = newWidth;
    height = newHeight;
    newWidth = (width / 2);
    newHeight = (height / 2);
    source = target;
    targetBufferIndex = 1 - targetBufferIndex;
  }

  /// Blur the image
  size = Vec2(float(width), float(height));
  globals->GBufferSourceA = renderTarget->mGBufferA;
  globals->PPGaussRelativeSize = Vec2(float(width) / float(originalWidth),
                                      float(height) / float(originalHeight));
  globals->PPGaussPixelSize = Vec2(1.0f, 1.0f) / renderTarget->GetSize();

  UINT gaussIterationCount = 10;

  for (UINT i = 0; i < gaussIterationCount * 2; i++) {
    FrameBufferId targetBuffer = renderTarget->mGaussFramebuffers[targetBufferIndex];
    OpenGL->SetFrameBuffer(targetBuffer);
    globals->PPGauss = renderTarget->mGaussTextures[1 - targetBufferIndex];
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
    targetBufferIndex = 1 - targetBufferIndex;
  }

  /// Blend to original image and perform HDR multisampling correction

  OpenGL->SetFrameBuffer(renderTarget->mColorBufferId);
  size = renderTarget->GetSize();
  glViewport(0, 0, originalWidth, originalHeight);
  globals->PPGauss = renderTarget->mGaussTextures[1 - targetBufferIndex];
  mPostProcess_GaussianBlur_Blend.Set(globals);
  mFullScreenQuad->Render(mPostProcess_GaussianBlur_Blend.GetUsedAttributes(), 1,
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
  StubNode* gaussianBlend =
    TheEngineStubs->GetStub("postprocess/gaussianblur-blend");

  mPostProcess_GaussianBlurHorizontal.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal.mFragmentStub.Connect(gaussianHorizontal);
  mPostProcess_GaussianBlurHorizontal.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurHorizontal.mBlendModeSlot.SetDefaultValue(1.0f); // normal
  mPostProcess_GaussianBlurHorizontal.mFaceModeSlot.SetDefaultValue(0.5f); // f&b
  //mPostProcess_GaussianBlurHorizontal.mRenderstate.mBlendMode =
  //  RenderState::BlendMode::NORMAL;
  //mPostProcess_GaussianBlurHorizontal.mRenderstate.mFaceMode =
  //  RenderState::FaceMode::FRONT_AND_BACK;

  mPostProcess_GaussianBlurHorizontal_First.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal_First.mFragmentStub.Connect(
    gaussianHorizontalFirst);
  mPostProcess_GaussianBlurHorizontal_First.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurHorizontal_First.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlurHorizontal_First.mFaceModeSlot.SetDefaultValue(0.5f);

  //mPostProcess_GaussianBlurHorizontal_First.mRenderstate.mBlendMode =
  //  RenderState::BlendMode::NORMAL;
  //mPostProcess_GaussianBlurHorizontal_First.mRenderstate.mFaceMode =
  //  RenderState::FaceMode::FRONT_AND_BACK;

  mPostProcess_GaussianBlurVertical.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurVertical.mFragmentStub.Connect(gaussianVertical);
  mPostProcess_GaussianBlurVertical.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlurVertical.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlurVertical.mFaceModeSlot.SetDefaultValue(0.5f);

  //mPostProcess_GaussianBlurVertical.mRenderstate.mBlendMode =
  //  RenderState::BlendMode::NORMAL;
  //mPostProcess_GaussianBlurVertical.mRenderstate.mFaceMode =
  //  RenderState::FaceMode::FRONT_AND_BACK;

  mPostProcess_GaussianBlur_Blend.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlur_Blend.mFragmentStub.Connect(gaussianBlend);
  mPostProcess_GaussianBlur_Blend.mRenderstate.mDepthTest = false;
  mPostProcess_GaussianBlur_Blend.mBlendModeSlot.SetDefaultValue(1.0f);
  mPostProcess_GaussianBlur_Blend.mFaceModeSlot.SetDefaultValue(0.5f);

  //mPostProcess_GaussianBlur_Blend.mRenderstate.mBlendMode =
  //  RenderState::BlendMode::NORMAL;
  //mPostProcess_GaussianBlur_Blend.mRenderstate.mFaceMode =
  //  RenderState::FaceMode::FRONT_AND_BACK;

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
