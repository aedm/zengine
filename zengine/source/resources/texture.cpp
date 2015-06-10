#include <include/resources/texture.h>

Texture::Texture( int width, int height, TexelTypeEnum type, TextureHandle handle )
	: mType(type)
	, mWidth(width)
	, mHeight(height)
	, mHandle(handle)
{}

Texture::~Texture()
{
}
