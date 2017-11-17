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
  for (Mesh* mesh : mMeshes) delete mesh;
  for (auto vertexFormat : mVertexFormats) delete vertexFormat.second;
}

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
