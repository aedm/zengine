# Zengine

A general 3D engine.


## How to build

1. Install dependencies: `vcpkg install glew glm assimp rapidjson qt5` 
2. Build in Visual Studio 2019.


#### Resolving build issues

1. Make sure `vcpkg` and Visual Studio 2019 use the same `cmake` version. Preferably the one shipped with VS2019.
1. Use static libraries: `˙set VCPKG_DEFAULT_TRIPLET=x64-windows-static`
