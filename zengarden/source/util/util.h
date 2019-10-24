#pragma once

#include <zengine.h>
#include <QtCore/QString>

namespace Util {

  /// Creates a shader pass
  std::shared_ptr<Pass> LoadShader(const char* VertexFile, const char* FragmentFile);
  std::shared_ptr<StubNode> LoadStub(const QString& fileName);

  /// Reads in a file using Qt
  OWNERSHIP char* ReadFileQt(const char* FileName);
  OWNERSHIP char* ReadFileQt(const QString& FileName);

  /// Loads a static mesh from a Wavefront .obj file
  std::shared_ptr<Mesh> LoadMesh(const QString& fileName);

  /// Disposes a set of nodes
  void DisposeNodes(const std::set<std::shared_ptr<Node>>& nodes);
}
