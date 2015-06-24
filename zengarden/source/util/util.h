#pragma once

#include <zengine.h>
#include <QtCore/QString>

namespace Util {

  /// Creates a shader pass
  Pass*	LoadShader(const char* VertexFile, const char* FragmentFile);

  /// Reads in a file using Qt
  OWNERSHIP char*	ReadFileQt(const char* FileName);
  OWNERSHIP char*	ReadFileQt(const QString& FileName);
}
