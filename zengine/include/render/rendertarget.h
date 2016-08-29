#pragma once

#include <include/resources/texture.h>

class GBuffer {
public:
  GBuffer();

  /// 0. [D24S8] depth / stencil
  Texture* mGBufferDepthStencil;
  /// 1. [4x16F] RGB: HDR final color, A: unused
  Texture* mGBuffer1;
  /// 2. [1x32] R: Last chain link in A-buffer linked list
  Texture* mGBuffer2;
};


class RenderTarget {
public:
  RenderTarget();
  ~RenderTarget();

  /// Sets up render chain with this render target
  /// writeDepth: if true, depth buffer is a render target, otherwise depth is a source texture
  /// writeGBuffer: if true, G-buffer is a render target, otherwise G-buffer textures are sampler sources
  void Set(bool writeDepth = true, bool writeGBuffer = false);

private:
  /// G-buffer textures:
  GBuffer* mGBuffer;

  /// A-buffer, holds OIT fragments
  /// ARGB 16F: translucent emissive color (alpha blend)
  /// RGB 16F: premultiplied light color (additive blend)
  /// D24: fragment depth
  /// R32: previous A-buffer link index
  Texture* mABuffer;
};
