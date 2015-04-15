#pragma once

#include "mesh.h"
#include "texture.h"

#include <map>
#include <set>

using namespace std;

class ResourceManager;
extern ResourceManager* TheResourceManager;

//typedef vector<Texture*> TextureList;

class ResourceManager
{
	//friend class XMLSerializer;
	//friend class XMLDeserializer;

public:
	ResourceManager();
	~ResourceManager();

	Texture*						CreateTexture(int Width, int Height, TexelTypeEnum Type, void* TexelData);
	void							DiscardTexture(Texture* TextureInstance);

	//ShaderSource*					CreateShaderSource(ShaderType Type, const char* Source);

	Mesh*							CreateMesh();
	void							DiscardMesh(Mesh* MeshInstance);

	VertexFormat*					GetVertexFormat(UINT BinaryFormat);

private:
	//struct TexCategory
	//{
	//	int							Width;
	//	int							Height;
	//	TextureType					Type;

	//	TexCategory(int Width, int Height, TextureType Type);
	//	bool operator < (const TexCategory& Category) const;
	//};

	//TextureList						AllTextures;
	//vector<ShaderSource*>			AllShaderSources;

	map<UINT, VertexFormat*>		VertexFormats;
	set<Mesh*>						Meshes;

	//map<TexCategory, TextureList*>	FreeTextures;
};
