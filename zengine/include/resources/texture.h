#pragma once

#include "../base/defines.h"

/// Sampler usages
enum SamplerUsageEnum
{
	SAMPLERUSAGE_LOCAL,								/// Local sampler
	SAMPLERUSAGE_NOISE
};

/// Texture map
class Texture
{
	friend class ResourceManager;

protected:
	Texture(int Width, int Height, TexelTypeEnum Type, TextureHandle Handle);
	~Texture();

public:
	const TexelTypeEnum		Type;

	const int				Width;
	const int				Height;

	const TextureHandle		Handle;
	//RenderTarget*			RenderTargetInstance;
};

