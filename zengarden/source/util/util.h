#pragma once

#include <zengine.h>
#include <QtCore/QString>

namespace Util {

  /// Creates a shader pass
  Pass*	LoadShader(const char* VertexFile, const char* FragmentFile);

  /// Reads in a file using Qt
  OWNERSHIP char*	ReadFileQt(const char* FileName);
  OWNERSHIP char*	ReadFileQt(const QString& FileName);

  /// Traverses the graph and generates a topological ordering. Deepest nodes come
  /// first. 
  void CreateTopologicalOrder(Node* root, vector<Node*>& oResult);

  /// Loads a static mesh from a Wavefront .obj file
  OWNERSHIP Mesh* LoadMesh(const QString& fileName);
}
