#include <include/resources/texture.h>
#include <include/render/drawingapi.h>
#include <include/base/helpers.h>
#include <utility>

Texture::~Texture() {
  OpenGLAPI::DeleteTextureGpuData(mHandle);
}

Texture::Texture(Handle handle, int width, int height, TexelType type,
                 std::shared_ptr<std::vector<char>> texelData, bool isMultisample, 
  bool doesRepeat, bool generateMipmaps)
  : mHandle(handle)
  , mWidth(width)
  , mHeight(height)
  , mType(type)
  , mIsMultisample(isMultisample)
  , mDoesRepeat(doesRepeat)
  , mGenerateMipmaps(generateMipmaps)
  , mTexelData(std::move(texelData))
{}
