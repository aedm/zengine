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


  /// -------------- VERTEX FORMAT MANAGEMENT --------------

  VertexFormat* GetVertexFormat(UINT binaryFormat);

private:
  map<UINT, VertexFormat*> mVertexFormats;
};
