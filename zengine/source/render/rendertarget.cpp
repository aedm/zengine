#include <include/render/rendertarget.h>
#include <include/base/helpers.h>
#include <include/resources/resourcemanager.h>
#include <glew/glew.h>


RenderTarget::RenderTarget(Vec2 size)
  : mSize(0,0)
{
  Resize(size);
}

RenderTarget::~RenderTarget() {
  DropResources();
  if (mColorBufferId) TheDrawingAPI->DeleteFrameBuffer(mColorBufferId);
}

void RenderTarget::SetGBufferAsTarget(Globals* globals) {
  globals->DepthBufferSource = 0;
  globals->GBufferSourceA = 0;
  TheDrawingAPI->SetFrameBuffer(mGBufferId);
}

void RenderTarget::SetColorBufferAsTarget(Globals* globals) {
  globals->DepthBufferSource = mDepthBuffer;
  globals->GBufferSourceA = mGBufferA;
  TheDrawingAPI->SetFrameBuffer(mColorBufferId);
}

void RenderTarget::Resize(Vec2 size) {
  if (size == mSize) return;
  GLenum error = glGetError();
  ASSERT(error == GL_NO_ERROR);
  DropResources();
  error = glGetError(); ASSERT(error == GL_NO_ERROR);

  mSize = size;
  UINT width = UINT(size.x);
  UINT height = UINT(size.y);

  /// Create G-Buffer
  mDepthBuffer = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::DEPTH32F, nullptr, false, false);
  mGBufferA = TheResourceManager->CreateGPUTexture(
    width, height, TexelType::ARGB16F, nullptr, false, false);
  mGBufferId = TheDrawingAPI->CreateFrameBuffer(mDepthBuffer->mHandle, mGBufferA->mHandle, 0);

  /// Create gaussian ping-pong textures
  for (int i = 0; i < 2; i++) {
    mGaussTextures[i] = TheResourceManager->CreateGPUTexture(
      width, height, TexelType::ARGB16F, nullptr, false, false);
    mGaussFramebuffers[i] = TheDrawingAPI->CreateFrameBuffer(0, mGaussTextures[i]->mHandle, 0);
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
