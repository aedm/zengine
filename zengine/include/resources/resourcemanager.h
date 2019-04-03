#pragma once

#include "mesh.h"
#include "texture.h"

#include <map>
#include <set>

using namespace std;

class ResourceManager;
extern ResourceManager* TheResourceManager;

class ResourceManager {
public:
  ResourceManager();
  ~ResourceManager();

  /// -------------- TEXTURE MANAGEMENT --------------

  /// Creates texture object. Normally texture objects store texel data in main memory
  /// in order to be serializable
  //Texture* CreateTexture(int width, int height, TexelType type, void* texelData);
  
  /// Creates texture with GPU-only texel data that can't be serialized
  //Texture* CreateGPUTexture(int width, int height, TexelType type, void* texelData, 
  //                          bool multiSample, bool doesRepeat);

  /// Frees texture resources and deletes Texture object
  /// TODO: this should be done by the destructor instead
  //void DiscardTexture(Texture* texture);


  /// -------------- MESH MANAGEMENT --------------

  /// Creates a mesh object
  Mesh* CreateMesh();

  /// Frees mesh resources and deletes Mesh object
  /// TODO: this should be done by the destructor instead
  void DiscardMesh(Mesh* mesh);


  /// -------------- SHADER PROGRAM MANAGEMENT --------------



  /// -------------- VERTEX FORMAT MANAGEMENT --------------

  VertexFormat* GetVertexFormat(UINT binaryFormat);

private:
  map<UINT, VertexFormat*> mVertexFormats;
  set<Mesh*> mMeshes;
};
