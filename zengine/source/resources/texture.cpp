#include <include/resources/texture.h>

Texture::Texture( int Width_, int Height_, TexelTypeEnum Type_, TextureHandle Handle_ )
	: mType(Type_)
	, mWidth(Width_)
	, mHeight(Height_)
	, mHandle(Handle_)
{}

Texture::~Texture()
{
}
