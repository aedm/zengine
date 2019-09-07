#pragma once

#include <vector>
#include "../resources/texture.h"

using namespace std;

namespace Zengine {
  void InitGDIPlus();
  shared_ptr<Texture> LoadTextureFromFile(const wstring& fileName);
}

