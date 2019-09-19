#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>

#define GLEW_STATIC
#include <glew/glew.h>

static const UINT BloomEffectMaxResolution = 256;

EngineShaders::EngineShaders() 
  : mSolidShadowPass(make_shared<Pass>())
{
  BuildPostProcessPasses();
  BuildMaterialPasses();
}

EngineShaders::~EngineShaders() {
  mPostProcess_GaussianBlurHorizontal_First->Dispose();
  mPostProcess_GaussianBlurHorizontal->Dispose();
  mPostProcess_GaussianBlurVertical->Dispose();
  mPostProcess_GaussianBlur_Blend_MSAA->Dispose();
  mPostProcess_DOF->Dispose();
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
  const Vec2 size = renderTarget->GetSize();
  const UINT width = UINT(size.x);
  const UINT height = UINT(size.y);

  /// Blit G-Buffer into postprocess ping-pong buffers 
  OpenGL->BlitFrameBuffer(renderTarget->mGBufferId,
                          renderTarget->GetPostprocessTargetFramebufferId(),
                          0, 0, width, height, 0, 0, width, height);
  renderTarget->SwapPostprocessBuffers();
}


void EngineShaders::ApplyDepthOfField(RenderTarget* renderTarget, Globals* globals) const
{
  mPostProcess_DOF->Update();
  if (!mPostProcess_DOF->isComplete()) return;

  const Vec2 size = renderTarget->GetSize();
  const UINT width = UINT(size.x);
  const UINT height = UINT(size.y);

  OpenGL->SetFrameBuffer(renderTarget->mDOFBufferId);
  glViewport(0, 0, width, height);
  globals->GBufferSourceA = renderTarget->mGBufferA;
  globals->DepthBufferSource = renderTarget->mDepthBuffer;

  mPostProcess_DOF->Set(globals);
  mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);

  OpenGL->BlitFrameBuffer(renderTarget->mDOFBufferId,
                          renderTarget->GetPostprocessTargetFramebufferId(),
                          0, 0, width, height, 0, 0, width, height);
  renderTarget->SwapPostprocessBuffers();
}

void EngineShaders::GenerateBloomTexture(RenderTarget* renderTarget, Globals* globals) {
  mPostProcess_GaussianBlurHorizontal_First->Update();
  if (!mPostProcess_GaussianBlurHorizontal_First->isComplete()) return;
  mPostProcess_GaussianBlurHorizontal->Update();
  if (!mPostProcess_GaussianBlurHorizontal->isComplete()) return;
  mPostProcess_GaussianBlurVertical->Update();
  if (!mPostProcess_GaussianBlurVertical->isComplete()) return;

  Vec2 size = renderTarget->GetSize();
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);
  const UINT originalWidth = width;
  const UINT originalHeight = height;

  const UINT downsampleCount = UINT(ceilf(log2f(size.x / float(BloomEffectMaxResolution))));

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

  const UINT gaussIterationCount = 1;

  for (UINT i = 0; i < gaussIterationCount * 2; i++) {
    OpenGL->SetFrameBuffer(renderTarget->GetPostprocessTargetFramebufferId());
    globals->PPGauss = renderTarget->GetPostprocessSourceTexture();
    shared_ptr<Pass> pass = (i % 2 == 0)
      ? mPostProcess_GaussianBlurHorizontal : mPostProcess_GaussianBlurVertical;
    if (i == 0) {
      pass = mPostProcess_GaussianBlurHorizontal_First;
    }
    if (i <= 1) {
      glViewport(0, 0, originalWidth, originalHeight);
      OpenGL->Clear(true, false, 0);
      glViewport(0, 0, width, height);
    }
    pass->Set(globals);
    mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);
    renderTarget->SwapPostprocessBuffers();
  }
}


void EngineShaders::RenderFinalImage(RenderTarget* renderTarget, Globals* globals,
  const shared_ptr<Texture>& sourceColorMSAA) const
{
  mPostProcess_GaussianBlur_Blend_MSAA->Update();
  if (!mPostProcess_GaussianBlur_Blend_MSAA->isComplete()) return;

  /// Additively blend bloom to Gbuffer, and perform HDR multisampling correction
  const Vec2 size = renderTarget->GetSize();
  const UINT width = UINT(size.x);
  const UINT height = UINT(size.y);
  OpenGL->SetFrameBuffer(renderTarget->mColorBufferId);
  glViewport(0, 0, width, height);
  globals->PPGauss = renderTarget->GetPostprocessSourceTexture();
  globals->GBufferSourceA = sourceColorMSAA;
  mPostProcess_GaussianBlur_Blend_MSAA->Set(globals);
  mFullScreenQuad->Render(1, PRIMITIVE_TRIANGLES);
}

void ConnectQuadPass(shared_ptr<Pass>& pass, shared_ptr<StubNode>& vertexStub,
  shared_ptr<StubNode>& fragmentStub)
{
  pass->mVertexStub.Connect(vertexStub);
  pass->mFragmentStub.Connect(fragmentStub);
  pass->mRenderstate.mDepthTest = false;
  pass->mBlendModeSlot.SetDefaultValue(1.0f); // normal
  pass->mFaceModeSlot.SetDefaultValue(0.5f); // front & back
  pass->Update();
}

void EngineShaders::BuildPostProcessPasses() {
  shared_ptr<StubNode> fullscreenVertex =
    TheEngineStubs->GetStub("postprocess/fullscreen-vertex");
  shared_ptr<StubNode> gaussianHorizontalFirst =
    TheEngineStubs->GetStub("postprocess/gaussianblur-horizontal-first");
  shared_ptr<StubNode> gaussianHorizontal =
    TheEngineStubs->GetStub("postprocess/gaussianblur-horizontal");
  shared_ptr<StubNode> gaussianVertical =
    TheEngineStubs->GetStub("postprocess/gaussianblur-vertical");
  shared_ptr<StubNode> gaussianBlendMSAA =
    TheEngineStubs->GetStub("postprocess/gaussianblur-blend-msaa");
  shared_ptr<StubNode> dofFragment =
    TheEngineStubs->GetStub("postprocess/depth-of-field");

  ConnectQuadPass(mPostProcess_GaussianBlurHorizontal, 
    fullscreenVertex, gaussianHorizontal);
  ConnectQuadPass(mPostProcess_GaussianBlurHorizontal_First,
    fullscreenVertex, gaussianHorizontalFirst);
  ConnectQuadPass(mPostProcess_GaussianBlurVertical,
    fullscreenVertex, gaussianVertical);
  ConnectQuadPass(mPostProcess_GaussianBlur_Blend_MSAA,
    fullscreenVertex, gaussianBlendMSAA);
  ConnectQuadPass(mPostProcess_DOF,
    fullscreenVertex, dofFragment);

  shared_ptr<StubNode> fluidVertexShader =
    TheEngineStubs->GetStub("fluid/vertex");
  shared_ptr<StubNode> curlFS = 
    TheEngineStubs->GetStub("fluid/curl-fragment");
  shared_ptr<StubNode> vorticityFS = 
    TheEngineStubs->GetStub("fluid/vorticity-fragment");
  shared_ptr<StubNode> divergenceFS = 
    TheEngineStubs->GetStub("fluid/divergence-fragment");
  shared_ptr<StubNode> fadeoutFS = 
    TheEngineStubs->GetStub("fluid/fadeout-fragment");
  shared_ptr<StubNode> pressureFS = 
    TheEngineStubs->GetStub("fluid/pressure-fragment");
  shared_ptr<StubNode> gradientFS = 
    TheEngineStubs->GetStub("fluid/gradientSubtract-fragment");
  shared_ptr<StubNode> advectionFS = 
    TheEngineStubs->GetStub("fluid/advection-fragment");

  ConnectQuadPass(mFluid_CurlPass, fluidVertexShader, curlFS);
  ConnectQuadPass(mFluid_VorticityPass, fluidVertexShader, vorticityFS);
  ConnectQuadPass(mFluid_DivergencePass, fluidVertexShader, divergenceFS);
  ConnectQuadPass(mFluid_FadeOutPass, fluidVertexShader, fadeoutFS);
  ConnectQuadPass(mFluid_PressurePass, fluidVertexShader, pressureFS);
  ConnectQuadPass(mFluid_GradientSubtractPass, fluidVertexShader, gradientFS);
  ConnectQuadPass(mFluid_AdvectionPass, fluidVertexShader, advectionFS);

  /// Fullscreen quad
  IndexEntry quadIndices[] = {0, 1, 2, 2, 1, 3};
  VertexPosUV quadVertices[] = {
    {Vec3(-1, -1, 0), Vec2(0, 0)},
    {Vec3(1, -1, 0), Vec2(1, 0)},
    {Vec3(-1, 1, 0), Vec2(0, 1)},
    {Vec3(1, 1, 0), Vec2(1, 1)},
  };

  mFullScreenQuad = make_shared<Mesh>();
  mFullScreenQuad->SetIndices(quadIndices);
  mFullScreenQuad->SetVertices(quadVertices);
}

void EngineShaders::BuildMaterialPasses() const
{
  mSolidShadowPass->mVertexStub.Connect(
    TheEngineStubs->GetStub("material/solid/shadowPass-vertex"));
  mSolidShadowPass->mFragmentStub.Connect(
    TheEngineStubs->GetStub("material/solid/shadowPass-fragment"));

  mSolidShadowPass->mRenderstate.mDepthTest = true;
  mSolidShadowPass->mBlendModeSlot.SetDefaultValue(0.0f); // normal
  mSolidShadowPass->mFaceModeSlot.SetDefaultValue(1.0f); // back
}
