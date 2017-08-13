#include <include/resources/resourcemanager.h>

ResourceManager::ResourceManager() {
  VertexPos::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK);
  VertexPosNorm::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK |
                                          VERTEXATTRIB_NORMAL_MASK);
  VertexPosUVNorm::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK |
                                            VERTEXATTRIB_NORMAL_MASK |
                                            VERTEXATTRIB_TEXCOORD_MASK);
  VertexPosUVNormTangent::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK |
                                                   VERTEXATTRIB_NORMAL_MASK |
                                                   VERTEXATTRIB_TEXCOORD_MASK |
                                                   VERTEXATTRIB_TANGENT_MASK);
  VertexPosUV::format = GetVertexFormat(VERTEXATTRIB_POSITION_MASK |
                                        VERTEXATTRIB_TEXCOORD_MASK);
}

ResourceManager::~ResourceManager() {
  //for (UINT i=0; i<AllTextures.size(); i++) delete AllTextures[i];
  //for (UINT i=0; i<AllShaderSources.size(); i++) delete AllShaderSources[i];
  //for (set<Mesh*>::iterator it = Meshes.begin(); it != Meshes.end(); it++) delete *it;

  //for (map<TexCategory, TextureList*>::iterator it = FreeTextures.begin(); it != FreeTextures.end(); it++)
  //{
  //	delete it->second;
  //}

  for (Mesh* mesh : mMeshes) delete mesh;
  for (auto vertexFormat : mVertexFormats) delete vertexFormat.second;
}

//Texture* ResourceManager::CreateTexture(int Width, int Height, TextureType Type, void* PixelData)
//{
//	TexCategory category(Width, Height, Type);
//	map<TexCategory, TextureList*>::iterator it = FreeTextures.find(category);
//
//	if (it != FreeTextures.end())
//	{
//		TextureList* textureList = it->second;
//		if (textureList->size() > 0)
//		{
//			Texture* texture = (*textureList)[textureList->size()-1];
//			textureList->pop_back();
//			return texture;
//		}
//	}
//
//	Texture* texture = new Texture(Width, Height, Type, PixelData);
//	AllTextures.push_back(texture);
//
//	return texture;
//}

//void ResourceManager::DiscardTexture( Texture* TextureInstance )
//{
//	if (TextureInstance)
//	{
//		TexCategory category(TextureInstance->Width, TextureInstance->Height, TextureInstance->Type);
//		map<TexCategory, TextureList*>::iterator it = FreeTextures.find(category);
//
//		if (it != FreeTextures.end())
//		{
//			it->second->push_back(TextureInstance);
//		}
//		else
//		{
//			TextureList* texList = new TextureList();
//			texList->push_back(TextureInstance);
//			FreeTextures[category] = texList;
//		}
//	}
//}

//ShaderSource* ResourceManager::CreateShaderSource( ShaderType Type, const char* Source )
//{
//	ShaderSource* shaderSource = new ShaderSource(Type, Source);
//	AllShaderSources.push_back(shaderSource);
//	return shaderSource;
//}

VertexFormat* ResourceManager::GetVertexFormat(UINT binaryFormat) {
  map<UINT, VertexFormat*>::iterator it = mVertexFormats.find(binaryFormat);
  if (it != mVertexFormats.end()) return it->second;

  VertexFormat* format = new VertexFormat(binaryFormat);
  mVertexFormats[binaryFormat] = format;

  return format;
}

Mesh* ResourceManager::CreateMesh() {
  Mesh* mesh = new Mesh();
  mMeshes.insert(mesh);
  return mesh;
}

void ResourceManager::DiscardMesh(Mesh* meshInstance) {
  mMeshes.erase(meshInstance);
  delete meshInstance;
}

Texture* ResourceManager::CreateTexture(int width, int height, TexelType type,
                                        OWNERSHIP void* texelData) {
  TextureHandle handle = OpenGL->CreateTexture(width, height, type, false, true, true);
  if (texelData) {
    OpenGL->UploadTextureData(handle, width, height, type, texelData);
    // TODO: error handling
  }
  Texture* texture = new Texture(width, height, type, handle, texelData, false);
  return texture;
}

Texture* ResourceManager::CreateGPUTexture(int width, int height, TexelType type,
                                           void* texelData, bool multiSample,
                                           bool doesRepeat) {
  TextureHandle handle =
    OpenGL->CreateTexture(width, height, type, multiSample, doesRepeat, false);
  if (texelData) {
    OpenGL->UploadTextureData(handle, width, height, type, texelData);
    // TODO: error handling
  }
  Texture* texture = new Texture(width, height, type, handle, nullptr, multiSample);
  return texture;
}

void ResourceManager::DiscardTexture(Texture* textureInstance) {
  if (!textureInstance) return;
  OpenGL->DeleteTexture(textureInstance->mHandle);
  delete textureInstance;
}
