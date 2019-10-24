#pragma once

#include <vector>
#include "../resources/texture.h"

namespace Zengine {
  void InitGDIPlus();
  std::shared_ptr<Texture> LoadTextureFromFile(const std::wstring& fileName);
}

