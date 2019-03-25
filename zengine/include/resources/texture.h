#pragma once

#include "../base/defines.h"
#include <memory>
#include <vector>

/// Texture map
class Texture {
public:
  typedef	UINT Handle;

  const Handle mHandle;
  const int	mWidth;
  const int mHeight;
  const TexelType mType;
  const bool mIsMultisample;
  const bool mDoesRepeat;
  const bool mGenerateMipmaps;
  const shared_ptr<vector<char>> mTexelData;

  ~Texture();

private:
  Texture(Handle handle, int width, int height, TexelType type,
    const shared_ptr<vector<char>> texelData, bool isMultisample, bool doesRepeat, 
    bool generateMipmaps);
};

