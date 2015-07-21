#pragma once

#include "../base/defines.h"

/// Texture map
class Texture {
  friend class ResourceManager;

protected:
  Texture(int width, int height, TexelTypeEnum type, TextureHandle handle, 
          OWNERSHIP void* texelData);
  ~Texture();

  static UINT GetTexelByteCount(TexelTypeEnum type);

public:
  const TexelTypeEnum mType;

  const int	mWidth;
  const int mHeight;

  const void* mTexelData;
  const UINT mTexelDataByteCount;

  const TextureHandle mHandle;
  //RenderTarget*			RenderTargetInstance;
};

