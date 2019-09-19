#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/render/drawingapi.h>

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

void RenderTarget::SetGBufferAsTarget(Globals* globals) const
{
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetGBufferAsTargetForZPostPass(Globals* globals) const
{
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = nullptr;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferForZPostPassId);
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) const
{
  OpenGL->SetFrameBuffer(mColorBufferId);
  globals->RenderTargetSize = mSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / mSize.x, 1.0f / mSize.y);
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGL->SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetShadowBufferAsTarget(Globals* globals) const
{
  globals->RenderTargetSize = Vec2(float(ShadowMapSize), float(ShadowMapSize));
  globals->RenderTargetSizeRecip =
    Vec2(1.0f / float(ShadowMapSize), 1.0f / float(ShadowMapSize));
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  globals->GBufferSampleCount = 1;
  globals->SecondaryTexture = nullptr;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGL->SetFrameBuffer(mShadowBufferId);
  OpenGL->SetViewport(0, 0, ShadowMapSize, ShadowMapSize);
}

void RenderTarget::SetSquareBufferAsTarget(Globals* globals) const
{
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
  const UINT width = UINT(size.x);
  const UINT height = UINT(size.y);

  /// Create G-Buffer
  mDepthBuffer = OpenGL->MakeTexture(width, height, TexelType::DEPTH32F, nullptr, true,
    true, false, false);
  mGBufferA = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    true, false, false);
  mGBufferId = OpenGL->CreateFrameBuffer(mDepthBuffer, mGBufferA, nullptr);
  mGBufferForZPostPassId = OpenGL->CreateFrameBuffer(nullptr, mGBufferA, nullptr);

  /// Create framebuffer for DOF result
  mDOFColorTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    true, false, false);
  mDOFBufferId = OpenGL->CreateFrameBuffer(nullptr, mDOFColorTexture, nullptr);

  /// Create secondary framebuffer, no MSAA
  mSecondaryTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    false, false, false);
  mSecondaryFramebuffer = OpenGL->CreateFrameBuffer(nullptr, mSecondaryTexture, nullptr);

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
    mShadowBufferId = OpenGL->CreateFrameBuffer(mShadowTexture, nullptr, nullptr);
  }

  /// Video output framebuffer
  if (mForFrameGrabbing && mColorBufferId == 0) {
    mShadowTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB8,
      nullptr, true, false, false, false);
    mColorBufferId = OpenGL->CreateFrameBuffer(nullptr, mColorTexture, nullptr);
  }

  /// Create gaussian ping-pong textures
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    // Don't multisample these
    mPostprocessTextures[i] = OpenGL->MakeTexture(width, height, TexelType::ARGB16F,
      nullptr, true, false, false, false);
    mPostprocessFramebuffers[i] = 
      OpenGL->CreateFrameBuffer(nullptr, mPostprocessTextures[i], nullptr);
  }
}

Vec2 RenderTarget::GetSize() const
{
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

void RenderTarget::FinishFrame() const
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
