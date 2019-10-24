#pragma once

#include "../base/defines.h"
#include <memory>
#include <vector>

/// Texture map
class Texture {
public:
  typedef	UINT Handle;

  Texture(Handle handle, int width, int height, TexelType type,
          std::shared_ptr<std::vector<char>> texelData, bool isMultisample, bool doesRepeat,
    bool generateMipmaps);
  ~Texture();

  const Handle mHandle;
  const int mWidth;
  const int mHeight;
  const TexelType mType;
  const bool mIsMultisample;
  const bool mDoesRepeat;
  const bool mGenerateMipmaps;
  const std::shared_ptr<std::vector<char>> mTexelData;
};

