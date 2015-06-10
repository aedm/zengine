#pragma once

#include "mesh.h"
#include "texture.h"

#include <map>
#include <set>

using namespace std;

class ResourceManager;
extern ResourceManager* TheResourceManager;

//typedef vector<Texture*> TextureList;

class ResourceManager {
  //friend class XMLSerializer;
  //friend class XMLDeserializer;

public:
  ResourceManager();
  ~ResourceManager();

  Texture* CreateTexture(int width, int height, TexelTypeEnum type, void* texelData);
  void DiscardTexture(Texture* texture);

  Mesh* CreateMesh();
  void DiscardMesh(Mesh* mesh);

  VertexFormat* GetVertexFormat(UINT binaryFormat);

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

  map<UINT, VertexFormat*> mVertexFormats;
  set<Mesh*> mMeshes;

  //map<TexCategory, TextureList*>	FreeTextures;
};
