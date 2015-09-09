#include <include/resources/texture.h>
#include <include/base/helpers.h>

Texture::Texture( int width, int height, TexelTypeEnum type, TextureHandle handle,
                 OWNERSHIP void* texelData)
	: mType(type)
	, mWidth(width)
	, mHeight(height)
	, mHandle(handle)
  , mTexelData(texelData)
  , mTexelDataByteCount(texelData ? width * height * GetTexelByteCount(type) : 0)
{}

Texture::~Texture() {
  delete mTexelData;
}

UINT Texture::GetTexelByteCount(TexelTypeEnum type) {
  switch (type) {
    case TEXELTYPE_VOID:
      return 0;
    case TEXELTYPE_ARGB8:
      return 4;
    case TEXELTYPE_ARGB16:
    case TEXELTYPE_ARGB16F:
      return 8;
    case TEXELTYPE_ARGB32F:
      return 16;
  }
  SHOULDNT_HAPPEN;
  return 0;
}
