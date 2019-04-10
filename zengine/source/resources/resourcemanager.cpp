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
