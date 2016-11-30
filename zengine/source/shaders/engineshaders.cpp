#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>
#include <include/resources/resourcemanager.h>

#define GLEW_STATIC
#include <glew/glew.h>

static const UINT BloomEffectMaxResolution = 256;

EngineShaders::EngineShaders() {
  BuildPostProcessPasses();
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
    glBlitNamedFramebuffer(source, target, 0, 0, width, height, 0, 0, newWidth, newHeight, 
                           GL_COLOR_BUFFER_BIT, GL_LINEAR);
    auto error = glGetError();  ASSERT(error == GL_NO_ERROR);
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
    TheDrawingAPI->SetFrameBuffer(targetBuffer);
    globals->PPGauss = renderTarget->mGaussTextures[1 - targetBufferIndex];
    Pass* pass = (i % 2 == 0)
      ? &mPostProcess_GaussianBlurHorizontal : &mPostProcess_GaussianBlurVertical;
    if (i == 0) {
      pass = &mPostProcess_GaussianBlurHorizontal_First;
    }
    if (i <= 1) {
      glViewport(0, 0, originalWidth, originalHeight);
      TheDrawingAPI->Clear(true, false, 0);
      glViewport(0, 0, width, height);
    }
    pass->Set(globals);
    mFullScreenQuad->Render(pass->GetUsedAttributes(), 1, PRIMITIVE_TRIANGLES);
    targetBufferIndex = 1 - targetBufferIndex;
  }

  /// Blend to original image and perform HDR multisampling correction
  
  TheDrawingAPI->SetFrameBuffer(renderTarget->mColorBufferId);
  size = renderTarget->GetSize();
  glViewport(0, 0, originalWidth, originalHeight);
  globals->PPGauss = renderTarget->mGaussTextures[1 - targetBufferIndex];
  mPostProcess_GaussianBlur_Blend.Set(globals);
  mFullScreenQuad->Render(mPostProcess_GaussianBlur_Blend.GetUsedAttributes(), 1, 
                          PRIMITIVE_TRIANGLES);


}

void EngineShaders::BuildPostProcessPasses() {
  StubNode* fullscreenVertex = TheEngineStubs->GetStub("postproc-fullscreen-vertex");
  StubNode* gaussianHorizontalFirst = TheEngineStubs->GetStub("postproc-gaussianblur-horizontal-first");
  StubNode* gaussianHorizontal = TheEngineStubs->GetStub("postproc-gaussianblur-horizontal");
  StubNode* gaussianVertical = TheEngineStubs->GetStub("postproc-gaussianblur-vertical");
  StubNode* gaussianBlend = TheEngineStubs->GetStub("postproc-gaussianblur-blend");

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

  mPostProcess_GaussianBlur_Blend.mVertexStub.Connect(fullscreenVertex);
  mPostProcess_GaussianBlur_Blend.mFragmentStub.Connect(gaussianBlend);
  mPostProcess_GaussianBlur_Blend.mRenderstate.BlendMode = RenderState::BLEND_NORMAL;
  mPostProcess_GaussianBlur_Blend.mRenderstate.DepthTest = false;
  mPostProcess_GaussianBlur_Blend.mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;

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
