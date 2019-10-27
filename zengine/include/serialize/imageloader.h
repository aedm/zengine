#pragma once

#include "../resources/texture.h"
#include <string>

namespace Zengine {
  void InitGDIPlus();
  std::shared_ptr<Texture> LoadTextureFromFile(const std::wstring& fileName);
}

