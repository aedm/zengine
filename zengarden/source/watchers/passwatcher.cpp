#include "passwatcher.h"
#include <memory>

PassWatcher::PassWatcher(const std::shared_ptr<Node>& pass)
  : GeneralSceneWatcher(pass) 
{
  mMaterial->mSolidPass.Connect(pass);

  /// Mesh
  const vec2 position(3.5f, 3.5f);
  const float w = 512, h = 512, u = 1, v = 1;
  IndexEntry boxIndices[] = {0, 1, 2, 2, 1, 3};
  VertexPosUv vertices[] = {
    {vec3(position.x, position.y, 0), vec2(0, 0)},
    {vec3(position.x + w, position.y, 0), vec2(u, 0)},
    {vec3(position.x, position.y + h, 0), vec2(0, v)},
    {vec3(position.x + w, position.y + h, 0), vec2(u, v)},
  };

  std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
  boxMesh->SetIndices(boxIndices);
  boxMesh->SetVertices(vertices);
  mMesh->Set(boxMesh);

  mDrawable->mMaterial.Connect(mMaterial);
  mDrawable->mMesh.Connect(mMesh);

  mTheScene->mDrawables.Connect(mDrawable);

  mTheScene->mCamera.GetNode()->mOrthonormal = true;
}
