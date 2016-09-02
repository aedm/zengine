#pragma once

#include "../base/defines.h"

/// Texture map
class Texture {
  friend class ResourceManager;

protected:
  Texture(int width, int height, TexelType type, TextureHandle handle, 
          OWNERSHIP void* texelData);
  ~Texture();

public:
  static UINT GetTexelByteCount(TexelType type);

  const TexelType mType;

  const int	mWidth;
  const int mHeight;

  const void* mTexelData;
  const UINT mTexelDataByteCount;

  const TextureHandle mHandle;
  //RenderTarget*			RenderTargetInstance;
};

