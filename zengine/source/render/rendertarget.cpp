#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/resources/resourcemanager.h>


static const int ShadowMapSize = 2048;

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
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  OpenGL->SetFrameBuffer(mColorBufferId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
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

  /// Create framebuffer for DOF result
  mDOFColorTexture = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, true, false);
  mDOFBufferId =
    OpenGL->CreateFrameBuffer(0, mDOFColorTexture->mHandle, 0, true);

  /// Create secondary framebuffer, no MSAA
  mSecondaryTexture = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, false, false);
  mSecondaryFramebuffer = 
    OpenGL->CreateFrameBuffer(0, mSecondaryTexture->mHandle, 0, false);

  /// Create shadow map
  if (mShadowBufferId == 0) {
    mShadowTexture = TheResourceManager->CreateGPUTexture(
      ShadowMapSize, ShadowMapSize, TexelType::DEPTH32F, nullptr, false, false);
    mShadowBufferId = 
      OpenGL->CreateFrameBuffer(mShadowTexture->mHandle, 0, 0, false);
  }

  /// Create gaussian ping-pong textures
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    // Don't multisample these
    mPostprocessTextures[i] = TheResourceManager->CreateGPUTexture(
      width, height, TexelType::ARGB16F, nullptr, false, false); 
    mPostprocessFramebuffers[i] = 
      OpenGL->CreateFrameBuffer(0, mPostprocessTextures[i]->mHandle, 0, false);
  }
}

Vec2 RenderTarget::GetSize() {
  return mSize;
}

FrameBufferId RenderTarget::GetPostprocessSourceFramebufferId() {
  return mPostprocessFramebuffers[1 - mPostprocessTargetBufferIndex];
}

FrameBufferId RenderTarget::GetPostprocessTargetFramebufferId() {
  return mPostprocessFramebuffers[mPostprocessTargetBufferIndex];
}

Texture* RenderTarget::GetPostprocessSourceTexture() {
  return mPostprocessTextures[1 - mPostprocessTargetBufferIndex];
}

void RenderTarget::SwapPostprocessBuffers() {
  mPostprocessTargetBufferIndex = 1 - mPostprocessTargetBufferIndex;
}

void RenderTarget::DropResources() {
  OpenGL->DeleteFrameBuffer(mGBufferId);
  TheResourceManager->DiscardTexture(mDepthBuffer);
  TheResourceManager->DiscardTexture(mGBufferA);
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    OpenGL->DeleteFrameBuffer(mPostprocessFramebuffers[i]);
    TheResourceManager->DiscardTexture(mPostprocessTextures[i]);
  }
}
