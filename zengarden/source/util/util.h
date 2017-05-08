#pragma once

#include <zengine.h>
#include <QtCore/QString>

namespace Util {

  /// Creates a shader pass
  Pass*	LoadShader(const char* VertexFile, const char* FragmentFile);
  StubNode* LoadStub(const QString& fileName);

  /// Reads in a file using Qt
  OWNERSHIP char*	ReadFileQt(const char* FileName);
  OWNERSHIP char*	ReadFileQt(const QString& FileName);

  /// Loads a static mesh from a Wavefront .obj file
  OWNERSHIP Mesh* LoadMesh(const QString& fileName);
}
