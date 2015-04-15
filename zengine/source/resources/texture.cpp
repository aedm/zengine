#include <include/resources/texture.h>

Texture::Texture( int Width_, int Height_, TexelTypeEnum Type_, TextureHandle Handle_ )
	: Type(Type_)
	, Width(Width_)
	, Height(Height_)
	, Handle(Handle_)
{}

Texture::~Texture()
{
}
