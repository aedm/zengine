#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/resources/resourcemanager.h>


static const int ShadowMapSize = 2048;
static const int SquareBufferSize = 1024;

RenderTarget::RenderTarget(Vec2 size, bool forFrameGrabbing)
  : mForFrameGrabbing(forFrameGrabbing)
{
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
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetGBufferAsTargetForZPostPass(Globals* globals) {
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = 0;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferForZPostPassId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) {
  OpenGL->SetFrameBuffer(mColorBufferId);
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetShadowBufferAsTarget(Globals* globals) {
  globals->RenderTargetSize = Vec2(float(ShadowMapSize), float(ShadowMapSize));
  globals->RenderTargetSizeRecip =
    Vec2(1.0f / float(ShadowMapSize), 1.0f / float(ShadowMapSize));
  globals->DepthBufferSource = 0;
  globals->GBufferSourceA = 0;
  globals->GBufferSampleCount = 1;
  globals->SecondaryTexture = 0;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGL->SetFrameBuffer(mShadowBufferId);
  OpenGL->SetViewport(0, 0, ShadowMapSize, ShadowMapSize);
}

void RenderTarget::SetSquareBufferAsTarget(Globals* globals) {
  globals->RenderTargetSize = Vec2(float(SquareBufferSize), float(SquareBufferSize));
  globals->RenderTargetSizeRecip =
    Vec2(1.0f / float(SquareBufferSize), 1.0f / float(SquareBufferSize));
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  OpenGL->SetFrameBuffer(mSquareFramebuffer);
  OpenGL->SetViewport(0, 0, SquareBufferSize, SquareBufferSize);
}

void RenderTarget::Resize(Vec2 size) {
  if (mForFrameGrabbing) {
    mScreenSize = size;
    size = mFrameGrabberSize;
  }

  if (size == mSize) return;
  DropResources();

  mSize = size;
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);

  /// Create G-Buffer
  mDepthBuffer = OpenGL->MakeTexture(width, height, TexelType::DEPTH32F, nullptr, true,
    true, false, false);
  mGBufferA = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    true, false, false);
  mGBufferId = OpenGL->CreateFrameBuffer(mDepthBuffer, mGBufferA, nullptr);
  mGBufferForZPostPassId = OpenGL->CreateFrameBuffer(0, mGBufferA, nullptr);

  /// Create framebuffer for DOF result
  mDOFColorTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    true, false, false);
  mDOFBufferId = OpenGL->CreateFrameBuffer(0, mDOFColorTexture, nullptr);

  /// Create secondary framebuffer, no MSAA
  mSecondaryTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    false, false, false);
  mSecondaryFramebuffer = OpenGL->CreateFrameBuffer(0, mSecondaryTexture, nullptr);

  /// Create square framebuffer, no MSAA
  mSquareDepthTexture = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize, 
    TexelType::DEPTH32F, nullptr, true, false, false, false);
  mSquareTexture1 = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mSquareTexture2 = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mSquareFramebuffer = OpenGL->CreateFrameBuffer(mSquareDepthTexture,
    mSquareTexture1, mSquareTexture2);

  /// Create shadow map
  if (mShadowBufferId == 0) {
    mShadowTexture = OpenGL->MakeTexture(ShadowMapSize, ShadowMapSize, 
      TexelType::DEPTH32F, nullptr, true, false, false, false);
    mShadowBufferId = OpenGL->CreateFrameBuffer(mShadowTexture, 0, 0);
  }

  /// Video output framebuffer
  if (mForFrameGrabbing && mColorBufferId == 0) {
    mShadowTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB8,
      nullptr, true, false, false, false);
    mColorBufferId = OpenGL->CreateFrameBuffer(0, mColorTexture, 0);
  }

  /// Create gaussian ping-pong textures
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    // Don't multisample these
    mPostprocessTextures[i] = OpenGL->MakeTexture(width, height, TexelType::ARGB16F,
      nullptr, true, false, false, false);
    mPostprocessFramebuffers[i] = 
      OpenGL->CreateFrameBuffer(0, mPostprocessTextures[i], 0);
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

shared_ptr<Texture> RenderTarget::GetPostprocessSourceTexture() {
  return mPostprocessTextures[1 - mPostprocessTargetBufferIndex];
}

void RenderTarget::SwapPostprocessBuffers() {
  mPostprocessTargetBufferIndex = 1 - mPostprocessTargetBufferIndex;
}

void RenderTarget::FinishFrame()
{
  if (mForFrameGrabbing) {
    OpenGL->BlitFrameBuffer(mColorBufferId, 0,
      0, 0, int(mFrameGrabberSize.x), int(mFrameGrabberSize.y),
      0, 0, int(mScreenSize.x), int(mScreenSize.y));
  }
}

void RenderTarget::DropResources() {
  OpenGL->DeleteFrameBuffer(mGBufferId);
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    OpenGL->DeleteFrameBuffer(mPostprocessFramebuffers[i]);
  }
}
