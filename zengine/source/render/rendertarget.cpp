#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/resources/resourcemanager.h>


static const int ShadowMapSize = 4096;

RenderTarget::RenderTarget(Vec2 size)
  : mSize(0, 0) {
  Resize(size);
}

RenderTarget::~RenderTarget() {
  DropResources();
  if (mColorBufferId) OpenGL->DeleteFrameBuffer(mColorBufferId);
}

void RenderTarget::SetGBufferAsTarget(Globals* globals) {
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = 0;
  globals->GBufferSourceA = 0;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) {
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  OpenGL->SetFrameBuffer(mColorBufferId);
}

void RenderTarget::SetShadowBufferAsTarget(Globals* globals) {
  globals->RenderTargetSize = Vec2(float(ShadowMapSize), float(ShadowMapSize));
  globals->RenderTargetSizeRecip = Vec2(1.0f / float(ShadowMapSize), 
                                        1.0f / float(ShadowMapSize));
  globals->DepthBufferSource = 0;
  globals->GBufferSourceA = 0;
  globals->GBufferSampleCount = 1;
  globals->SecondaryTexture = 0;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mShadowBufferId);
  OpenGL->SetViewport(0, 0, ShadowMapSize, ShadowMapSize);
}

void RenderTarget::Resize(Vec2 size) {
  if (size == mSize) return;
  DropResources();

  mSize = size;
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);

  /// Create G-Buffer
  mDepthBuffer = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::DEPTH32F, nullptr, true, false);
  mGBufferA = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, true, false);
  mGBufferId = 
    OpenGL->CreateFrameBuffer(mDepthBuffer->mHandle, mGBufferA->mHandle, 0, true);

  /// Create secondary framebuffer
  mSecondaryTexture = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, false, false);
  mSecondaryFramebuffer = 
    OpenGL->CreateFrameBuffer(0, mSecondaryTexture->mHandle, 0, false);

  /// Create shadow map
  if (mShadowBufferId == 0) {
    mShadowTexture = TheResourceManager->CreateGPUTexture(
      ShadowMapSize, ShadowMapSize, TexelType::DEPTH32F, nullptr, false, false);
    mShadowBufferId = OpenGL->CreateFrameBuffer(mShadowTexture->mHandle,
                                                0, 0, false);
  }

  /// Create gaussian ping-pong textures
  for (int i = 0; i < 3; i++) {
    mGaussTextures[i] = TheResourceManager->CreateGPUTexture(
      width, height, TexelType::ARGB16F, nullptr, false, false); // don't multisample these
    mGaussFramebuffers[i] = OpenGL->CreateFrameBuffer(0, mGaussTextures[i]->mHandle, 0, false);
  }
}

Vec2 RenderTarget::GetSize() {
  return mSize;
}

void RenderTarget::DropResources() {
  OpenGL->DeleteFrameBuffer(mGBufferId);
  TheResourceManager->DiscardTexture(mDepthBuffer);
  TheResourceManager->DiscardTexture(mGBufferA);
  for (int i = 0; i < 2; i++) {
    OpenGL->DeleteFrameBuffer(mGaussFramebuffers[i]);
    TheResourceManager->DiscardTexture(mGaussTextures[i]);
  }
}
