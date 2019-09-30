#pragma once

#include "../base/defines.h"
#include <memory>
#include <vector>

using namespace std;

/// Texture map
class Texture {
public:
  typedef	UINT Handle;

  Texture(Handle handle, int width, int height, TexelType type,
          shared_ptr<vector<char>> texelData, bool isMultisample, bool doesRepeat,
    bool generateMipmaps);
  ~Texture();

  const Handle mHandle;
  const int mWidth;
  const int mHeight;
  const TexelType mType;
  const bool mIsMultisample;
  const bool mDoesRepeat;
  const bool mGenerateMipmaps;
  const shared_ptr<vector<char>> mTexelData;
};

