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

  /// -------------- MESH MANAGEMENT --------------

  /// Creates a mesh object
  Mesh* CreateMesh();

  /// Frees mesh resources and deletes Mesh object
  /// TODO: this should be done by the destructor instead
  void DiscardMesh(Mesh* mesh);

  /// -------------- VERTEX FORMAT MANAGEMENT --------------

  VertexFormat* GetVertexFormat(UINT binaryFormat);

private:
  map<UINT, VertexFormat*> mVertexFormats;
  set<Mesh*> mMeshes;
};
