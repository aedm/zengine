#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/resources/resourcemanager.h>


RenderTarget::RenderTarget(Vec2 size)
  : mSize(0, 0) {
  Resize(size);
}

RenderTarget::~RenderTarget() {
  DropResources();
  if (mColorBufferId) TheDrawingAPI->DeleteFrameBuffer(mColorBufferId);
}

void RenderTarget::SetGBufferAsTarget(Globals* globals) {
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = 0;
  globals->GBufferSourceA = 0;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  TheDrawingAPI->SetFrameBuffer(mGBufferId);
  TheDrawingAPI->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) {
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  TheDrawingAPI->SetFrameBuffer(mColorBufferId);
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
  mGBufferId = TheDrawingAPI->CreateFrameBuffer(mDepthBuffer->mHandle, mGBufferA->mHandle, 0, true);

  /// Create secondary framebuffer
  mSecondaryTexture = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, false, false);
  mSecondaryFramebuffer = TheDrawingAPI->CreateFrameBuffer(0, mSecondaryTexture->mHandle, 0, false);

  /// Create shadow map
  mShadowTexture = TheResourceManager->CreateGPUTexture(
    512, 512, TexelType::DEPTH32F, nullptr, false, false);
  mShadowColorBuffer = TheResourceManager->CreateGPUTexture(
    512, 512, TexelType::ARGB16F, nullptr, false, false);
  mShadowBufferId = TheDrawingAPI->CreateFrameBuffer(mShadowTexture->mHandle, mShadowColorBuffer->mHandle, 0, false);

  /// Create gaussian ping-pong textures
  for (int i = 0; i < 3; i++) {
    mGaussTextures[i] = TheResourceManager->CreateGPUTexture(
      width, height, TexelType::ARGB16F, nullptr, false, false); // don't multisample these
    mGaussFramebuffers[i] = TheDrawingAPI->CreateFrameBuffer(0, mGaussTextures[i]->mHandle, 0, false);
  }
}

Vec2 RenderTarget::GetSize() {
  return mSize;
}

void RenderTarget::DropResources() {
  TheDrawingAPI->DeleteFrameBuffer(mGBufferId);
  TheResourceManager->DiscardTexture(mDepthBuffer);
  TheResourceManager->DiscardTexture(mGBufferA);
  for (int i = 0; i < 2; i++) {
    TheDrawingAPI->DeleteFrameBuffer(mGaussFramebuffers[i]);
    TheResourceManager->DiscardTexture(mGaussTextures[i]);
  }
}
