#pragma once

#include "../resources/texture.h"
#include "../shaders/stubnode.h"

class RenderTarget {
public:
  RenderTarget(Vec2 size);
  ~RenderTarget();

  /// Sets the G-Buffer as render target
  void SetGBufferAsTarget(Globals* globals);

  /// Sets the final color buffer as render target, G-Buffer as source
  void SetColorBufferAsTarget(Globals* globals);
  void SetShadowBufferAsTarget(Globals* globals);
  void SetSquareBufferAsTarget(Globals* globals);

  void Resize(Vec2 size);
  Vec2 GetSize();

  FrameBufferId GetPostprocessSourceFramebufferId();
  FrameBufferId GetPostprocessTargetFramebufferId();
  Texture* GetPostprocessSourceTexture();
  void SwapPostprocessBuffers();

  /// G-buffer (MSAA)
  /// A: [4x16F] RGB: HDR final color, A: unused
  /// B: [1x32] R: Last chain link in A-buffer linked list (todo)
  FrameBufferId mGBufferId = 0;
  Texture* mDepthBuffer = nullptr;
  Texture* mGBufferA = nullptr;

  /// Depth of field result texture (MSAA)
  FrameBufferId mDOFBufferId = 0;
  Texture* mDOFColorTexture = nullptr;

  /// Secondary texture. For some fx.
  FrameBufferId mSecondaryFramebuffer = 0;
  Texture* mSecondaryTexture = nullptr;

  /// Square target textures. For some fx.
  FrameBufferId mSquareFramebuffer = 0;
  Texture* mSquareDepthTexture = nullptr;
  Texture* mSquareTexture1 = nullptr;
  Texture* mSquareTexture2 = nullptr;

  /// Final color buffer (framebuffer or texture)
  /// Default framebuffer id is 0
  FrameBufferId mColorBufferId = 0;

  /// A-buffer, holds OIT fragments
  /// ARGB 16F: translucent emissive color (alpha blend)
  /// RGB 16F: premultiplied light color (additive blend)
  /// D24: fragment depth
  /// R32: previous A-buffer link index
  Texture* mABuffer = nullptr;

  /// Skylight shadow
  FrameBufferId mShadowBufferId = 0;
  Texture* mShadowTexture = nullptr;

private:
  Vec2 mSize;

  void DropResources();

  /// Gaussian half-resolution ping-pong buffers, for intermediate blurred images
  Texture* mPostprocessTextures[2] = {nullptr, nullptr};
  FrameBufferId mPostprocessFramebuffers[2] = {0, 0};

  /// Current postprocess draw framebuffer index (the other one is the source)
  int mPostprocessTargetBufferIndex = 0;


};
