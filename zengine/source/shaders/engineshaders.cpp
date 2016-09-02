#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>
#include <include/resources/resourcemanager.h>

#define GLEW_STATIC
#include <glew/glew.h>

EngineShaders::EngineShaders() {
  BuildPostProcessPasses();
}

EngineShaders::~EngineShaders() {
  TheResourceManager->DiscardMesh(mFullScreenQuad);
}

void EngineShaders::ApplyPostProcess(RenderTarget* renderTarget, Globals* globals) {
  UINT gaussIterationCount = 20;
  mPostProcess_GaussianBlurHorizontal_First.Update();
  mPostProcess_GaussianBlurHorizontal.Update();
  mPostProcess_GaussianBlurVertical.Update();
  mPostProcess_GaussianBlurVertical_Last.Update();
  if (!mPostProcess_GaussianBlurHorizontal.isComplete()
      || !mPostProcess_GaussianBlurVertical.isComplete()
      || !mPostProcess_GaussianBlurHorizontal_First.isComplete()
      || !mPostProcess_GaussianBlurVertical_Last.isComplete()) return;
  
  /// Blit G-Buffer into gaussian ping-pong buffers to decrease resolution
  UINT width = renderTarget->GetSize().x;
  UINT height = renderTarget->GetSize().y;

  globals->GBufferSourceA = renderTarget->mGBufferA;
  globals->RenderTargetSize = renderTarget->GetSize() / Vec2(2, 2);
  globals->RenderTargetSizeRecip = Vec2(0.5f, 0.5f) / renderTarget->GetSize();
  glViewport(0, 0, width / 2, height / 2);

  for (int i = 0; i < gaussIterationCount; i++) {
    /// Horizontal pass
    TheDrawingAPI->SetFrameBuffer(renderTarget->mGaussFramebuffers[0]);
    globals->PPGauss = renderTarget->mGaussTextures[1];
    Pass& horizontalPass = i == 0 
      ? mPostProcess_GaussianBlurHorizontal_First 
      : mPostProcess_GaussianBlurHorizontal;
    horizontalPass.Set(globals);
    mFullScreenQuad->Render(horizontalPass.GetUsedAttributes(), 1, PRIMITIVE_TRIANGLES);

    /// Vertical pass
    FrameBufferId frameBuffer = (i == gaussIterationCount - 1) 
      ? renderTarget->mColorBufferId 
      : renderTarget->mGaussFramebuffers[1];
    Pass& verticalPass = (i == gaussIterationCount - 1)
      ? mPostProcess_GaussianBlurVertical_Last
      : mPostProcess_GaussianBlurVertical;
    if (i == gaussIterationCount-1) {
      glViewport(0, 0, width, height);
    }
    globals->PPGauss = renderTarget->mGaussTextures[0];
    TheDrawingAPI->SetFrameBuffer(frameBuffer);
    verticalPass.Set(globals);
    mFullScreenQuad->Render(verticalPass.GetUsedAttributes(), 1, PRIMITIVE_TRIANGLES);
  }
}

void EngineShaders::BuildPostProcessPasses() {
  StubNode* fullscreenVertex = TheEngineStubs->GetStub("postproc-fullscreen-vertex");
  StubNode* gaussianHorizontalFirst = TheEngineStubs->GetStub("postproc-gaussianblur-horizontal-first");
  StubNode* gaussianHorizontal = TheEngineStubs->GetStub("postproc-gaussianblur-horizontal");
  StubNode* gaussianVertical = TheEngineStubs->GetStub("postproc-gaussianblur-vertical");
  StubNode* gaussianVerticalLast = TheEngineStubs->GetStub("postproc-gaussianblur-vertical-last");

  mPostProcess_GaussianBlurHorizontal.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal.mFragmentStub.Connect(gaussianHorizontal);
  mPostProcess_GaussianBlurHorizontal.mRenderstate.BlendMode = RenderState::BLEND_NORMAL;
  mPostProcess_GaussianBlurHorizontal.mRenderstate.DepthTest = false;
  mPostProcess_GaussianBlurHorizontal.mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;

  mPostProcess_GaussianBlurHorizontal_First.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurHorizontal_First.mFragmentStub.Connect(gaussianHorizontalFirst);
  mPostProcess_GaussianBlurHorizontal_First.mRenderstate.BlendMode = RenderState::BLEND_NORMAL;
  mPostProcess_GaussianBlurHorizontal_First.mRenderstate.DepthTest = false;
  mPostProcess_GaussianBlurHorizontal_First.mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;

  mPostProcess_GaussianBlurVertical.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurVertical.mFragmentStub.Connect(gaussianVertical);
  mPostProcess_GaussianBlurVertical.mRenderstate.BlendMode = RenderState::BLEND_NORMAL;
  mPostProcess_GaussianBlurVertical.mRenderstate.DepthTest = false;
  mPostProcess_GaussianBlurVertical.mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;

  mPostProcess_GaussianBlurVertical_Last.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlurVertical_Last.mFragmentStub.Connect(gaussianVerticalLast);
  mPostProcess_GaussianBlurVertical_Last.mRenderstate.BlendMode = RenderState::BLEND_NORMAL;
  mPostProcess_GaussianBlurVertical_Last.mRenderstate.DepthTest = false;
  mPostProcess_GaussianBlurVertical_Last.mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;

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
