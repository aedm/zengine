#include <include/resources/texture.h>
#include <include/base/helpers.h>

Texture::Texture( int width, int height, TexelType type, TextureHandle handle,
                 OWNERSHIP void* texelData, bool isRenderTarget)
	: mType(type)
	, mWidth(width)
	, mHeight(height)
	, mHandle(handle)
  , mTexelData(texelData)
  , mTexelDataByteCount(texelData ? width * height * GetTexelByteCount(type) : 0)
  , mIsMultisampe(isRenderTarget)
{}

Texture::~Texture() {
  delete mTexelData;
}

UINT Texture::GetTexelByteCount(TexelType type) {
  switch (type) {
    case TexelType::ARGB8:
    case TexelType::DEPTH32F:
      return 4;
    case TexelType::ARGB16:
    case TexelType::ARGB16F:
      return 8;
    case TexelType::ARGB32F:
      return 16;
  }
  SHOULDNT_HAPPEN;
  return 0;
}
