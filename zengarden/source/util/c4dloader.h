#pragma once
#include <zengine.h>
#include <QtCore/QDir>

namespace Util {
  OWNERSHIP Mesh* LoadC4DMesh(const QString& fileName);
}

