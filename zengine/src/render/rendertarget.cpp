#include <include/render/rendertarget.h>
#include <include/render/drawingapi.h>

static const int ShadowMapSize = 2048;
static const int SquareBufferSize = 1024;

RenderTarget::RenderTarget(ivec2 size, bool forFrameGrabbing)
  : mForFrameGrabbing(forFrameGrabbing)
{
  Resize(size);
}

RenderTarget::~RenderTarget() {
  DropResources();
  if (mColorBufferId) OpenGLAPI::DeleteFrameBuffer(mColorBufferId);
}

void RenderTarget::SetGBufferAsTarget(Globals* globals) const
{
  globals->RenderTargetSize = { mSize.x, mSize.y };
  globals->RenderTargetSizeRecip = 1.0f / globals->RenderTargetSize;
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferId);
  OpenGLAPI::SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetGBufferAsTargetForZPostPass(Globals* globals) const
{
  globals->RenderTargetSize = { mSize.x, mSize.y };
  globals->RenderTargetSizeRecip = 1.0f / globals->RenderTargetSize;
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = nullptr;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  globals->GBufferSampleCount = ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT;
  globals->SecondaryTexture = mSecondaryTexture;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  OpenGL->SetFrameBuffer(mGBufferForZPostPassId);
  OpenGLAPI::SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) const
{
  OpenGL->SetFrameBuffer(mColorBufferId);
  globals->RenderTargetSize = { mSize.x, mSize.y };
  globals->RenderTargetSizeRecip = 1.0f / globals->RenderTargetSize;
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGLAPI::SetViewport(0, 0, int(mSize.x), int(mSize.y));
}

void RenderTarget::SetShadowBufferAsTarget(Globals* globals) const
{
  globals->RenderTargetSize = { ShadowMapSize, ShadowMapSize };
  globals->RenderTargetSizeRecip = 1.0f / globals->RenderTargetSize;
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  globals->GBufferSampleCount = 1;
  globals->SecondaryTexture = nullptr;
  globals->SkylightTextureSizeRecip = 1.0f / float(ShadowMapSize);
  globals->SquareTexture1 = mSquareTexture1;
  globals->SquareTexture2 = mSquareTexture2;
  OpenGL->SetFrameBuffer(mShadowBufferId);
  OpenGLAPI::SetViewport(0, 0, ShadowMapSize, ShadowMapSize);
}

void RenderTarget::SetSquareBufferAsTarget(Globals* globals) const
{
  globals->RenderTargetSize = { SquareBufferSize, SquareBufferSize };
  globals->RenderTargetSizeRecip = 1.0f / globals->RenderTargetSize;
  globals->DepthBufferSource = nullptr;
  globals->GBufferSourceA = nullptr;
  OpenGL->SetFrameBuffer(mSquareFramebuffer);
  OpenGLAPI::SetViewport(0, 0, SquareBufferSize, SquareBufferSize);
}

void RenderTarget::Resize(ivec2 size) {
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
  mGBufferId = OpenGLAPI::CreateFrameBuffer(mDepthBuffer, mGBufferA, nullptr);
  mGBufferForZPostPassId = OpenGLAPI::CreateFrameBuffer(nullptr, mGBufferA, nullptr);

  /// Create framebuffer for DOF result
  mDOFColorTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    true, false, false);
  mDOFBufferId = OpenGLAPI::CreateFrameBuffer(nullptr, mDOFColorTexture, nullptr);

  /// Create secondary framebuffer, no MSAA
  mSecondaryTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB16F, nullptr, true,
    false, false, false);
  mSecondaryFramebuffer = OpenGLAPI::CreateFrameBuffer(nullptr, mSecondaryTexture, nullptr);

  /// Create square framebuffer, no MSAA
  mSquareDepthTexture = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize, 
    TexelType::DEPTH32F, nullptr, true, false, false, false);
  mSquareTexture1 = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mSquareTexture2 = OpenGL->MakeTexture(SquareBufferSize, SquareBufferSize,
    TexelType::ARGB16F, nullptr, true, false, false, false);
  mSquareFramebuffer = OpenGLAPI::CreateFrameBuffer(mSquareDepthTexture,
    mSquareTexture1, mSquareTexture2);

  /// Create shadow map
  if (mShadowBufferId == 0) {
    mShadowTexture = OpenGL->MakeTexture(ShadowMapSize, ShadowMapSize, 
      TexelType::DEPTH32F, nullptr, true, false, false, false);
    mShadowBufferId = OpenGLAPI::CreateFrameBuffer(mShadowTexture, nullptr, nullptr);
  }

  /// Video output framebuffer
  if (mForFrameGrabbing && mColorBufferId == 0) {
    mColorTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB8,
      nullptr, true, false, false, false);
    mColorBufferId = OpenGLAPI::CreateFrameBuffer(nullptr, mColorTexture, nullptr);
  }

  /// Create gaussian ping-pong textures
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    // Don't multisample these
    mPostprocessTextures[i] = OpenGL->MakeTexture(width, height, TexelType::ARGB16F,
      nullptr, true, false, false, false);
    mPostprocessFramebuffers[i] =
      OpenGLAPI::CreateFrameBuffer(nullptr, mPostprocessTextures[i], nullptr);
  }
}

ivec2 RenderTarget::GetSize() const
{
  return mSize;
}

FrameBufferId RenderTarget::GetPostprocessSourceFramebufferId() {
  return mPostprocessFramebuffers[1 - mPostprocessTargetBufferIndex];
}

FrameBufferId RenderTarget::GetPostprocessTargetFramebufferId() {
  return mPostprocessFramebuffers[mPostprocessTargetBufferIndex];
}

std::shared_ptr<Texture> RenderTarget::GetPostprocessSourceTexture() {
  return mPostprocessTextures[1 - mPostprocessTargetBufferIndex];
}

void RenderTarget::SwapPostprocessBuffers() {
  mPostprocessTargetBufferIndex = 1 - mPostprocessTargetBufferIndex;
}

void RenderTarget::FinishFrame() const
{
  if (mForFrameGrabbing) {
    OpenGLAPI::BlitFrameBuffer(mColorBufferId, 0,
      0, 0, int(mFrameGrabberSize.x), int(mFrameGrabberSize.y),
      0, 0, int(mScreenSize.x), int(mScreenSize.y));
  }
}

void RenderTarget::DropResources() {
  OpenGLAPI::DeleteFrameBuffer(mGBufferId);
  for (UINT i = 0; i < ElementCount(mPostprocessTextures); i++) {
    OpenGLAPI::DeleteFrameBuffer(mPostprocessFramebuffers[i]);
  }
}
