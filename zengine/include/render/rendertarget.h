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

  void Resize(Vec2 size);
  Vec2 GetSize();

  /// Gaussian ping-pong buffers, for intermediate blurred images
  Texture* mGaussTextures[2] = {nullptr, nullptr};
  FrameBufferId mGaussFramebuffers[2] = {0, 0};

  /// G-buffer
  /// A: [4x16F] RGB: HDR final color, A: unused
  /// B: [1x32] R: Last chain link in A-buffer linked list
  FrameBufferId mGBufferId = 0;
  Texture* mDepthBuffer = nullptr;
  Texture* mGBufferA = nullptr;

  /// Final color buffer (framebuffer or texture)
  /// Default frambuffer id is 0
  FrameBufferId mColorBufferId = 0;

  /// A-buffer, holds OIT fragments
  /// ARGB 16F: translucent emissive color (alpha blend)
  /// RGB 16F: premultiplied light color (additive blend)
  /// D24: fragment depth
  /// R32: previous A-buffer link index
  Texture* mABuffer = nullptr;

private:
  Vec2 mSize;

  void DropResources();
};
