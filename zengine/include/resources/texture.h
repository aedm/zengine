#pragma once

#include "../base/defines.h"

/// Texture map
class Texture {
  friend class ResourceManager;

protected:
  Texture(int width, int height, TexelTypeEnum type, TextureHandle handle);
  ~Texture();

public:
  const TexelTypeEnum mType;

  const int	mWidth;
  const int mHeight;

  const TextureHandle mHandle;
  //RenderTarget*			RenderTargetInstance;
};

