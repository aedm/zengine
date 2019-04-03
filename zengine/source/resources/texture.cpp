#include <include/resources/texture.h>
#include <include/render/drawingapi.h>
#include <include/base/helpers.h>

Texture::~Texture() {
  OpenGL->DeleteTextureGPUData(mHandle);
}

//Texture::Texture( int width, int height, TexelType type, TextureHandle handle,
//                 OWNERSHIP void* texelData, bool isRenderTarget)
//	: mType(type)
//	, mWidth(width)
//	, mHeight(height)
//	, mHandle(handle)
//  , mTexelData(texelData)
//  , mTexelDataByteCount(texelData ? width * height * GetTexelByteCount(type) : 0)
//  , mIsMultisampe(isRenderTarget)
//{}


Texture::Texture(Handle handle, int width, int height, TexelType type, 
  const shared_ptr<vector<char>> texelData, bool isMultisample, bool doesRepeat, 
  bool generateMipmaps)
  : mHandle(handle)
  , mWidth(width)
  , mHeight(height)
  , mType(type)
  , mTexelData(texelData)
  , mIsMultisample(isMultisample)
  , mDoesRepeat(doesRepeat)
  , mGenerateMipmaps(generateMipmaps)
{}

