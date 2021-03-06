#pragma once

#include "../resources/texture.h"
#include "../shaders/stubnode.h"

class RenderTarget {
public:
  RenderTarget(ivec2 size, bool forFrameGrabbing = false);
  ~RenderTarget();

  /// Sets the G-Buffer as render target
  void SetGBufferAsTarget(Globals* globals) const;
  void SetGBufferAsTargetForZPostPass(Globals* globals) const;

  /// Sets the final color buffer as render target, G-Buffer as source
  void SetColorBufferAsTarget(Globals* globals) const;
  void SetShadowBufferAsTarget(Globals* globals) const;
  void SetSquareBufferAsTarget(Globals* globals) const;

  void Resize(ivec2 size);
  ivec2 GetSize() const;

  FrameBufferId GetPostprocessSourceFramebufferId();
  FrameBufferId GetPostprocessTargetFramebufferId();
  std::shared_ptr<Texture> GetPostprocessSourceTexture();
  void SwapPostprocessBuffers();
  void FinishFrame() const;

  /// G-buffer (MSAA)
  /// A: [4x16F] RGB: HDR final color, A: unused
  /// B: [1x32] R: Last chain link in A-buffer linked list (todo)
  FrameBufferId mGBufferId = 0;
  FrameBufferId mGBufferForZPostPassId = 0;
  std::shared_ptr<Texture> mDepthBuffer = nullptr;
  std::shared_ptr<Texture> mGBufferA = nullptr;

  /// Depth of field result texture (MSAA)
  FrameBufferId mDOFBufferId = 0;
  std::shared_ptr<Texture> mDOFColorTexture = nullptr;

  /// Secondary texture. For some fx.
  FrameBufferId mSecondaryFramebuffer = 0;
  std::shared_ptr<Texture> mSecondaryTexture = nullptr;

  /// Square target textures. For some fx.
  FrameBufferId mSquareFramebuffer = 0;
  std::shared_ptr<Texture> mSquareDepthTexture = nullptr;
  std::shared_ptr<Texture> mSquareTexture1 = nullptr;
  std::shared_ptr<Texture> mSquareTexture2 = nullptr;

  /// Final color buffer (framebuffer or texture)
  /// Default framebuffer id is 0
  FrameBufferId mColorBufferId = 0;

  /// When color buffer texture is created for frame grabbing
  std::shared_ptr<Texture> mColorTexture = nullptr;

  /// Skylight shadow
  FrameBufferId mShadowBufferId = 0;
  std::shared_ptr<Texture> mShadowTexture = nullptr;

  ivec2 mSize = { 0, 0 };
private:
  /// Screen size, can be different than render target size when grabbing frames
  ivec2 mScreenSize = { 0, 0 };

  const ivec2 mFrameGrabberSize = { 1920, 1080 };

  void DropResources();

  /// Gaussian half-resolution ping-pong buffers, for intermediate blurred images
  std::shared_ptr<Texture> mPostprocessTextures[2] = { nullptr, nullptr };
  FrameBufferId mPostprocessFramebuffers[2] = { 0, 0 };

  /// Current postprocess draw framebuffer index (the other one is the source)
  int mPostprocessTargetBufferIndex = 0;

  const bool mForFrameGrabbing;
};
