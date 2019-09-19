#include "passwatcher.h"

PassWatcher::PassWatcher(const shared_ptr<Node>& pass)
  : GeneralSceneWatcher(pass) 
{
  mMaterial->mSolidPass.Connect(pass);

  /// Mesh
  const Vec2 Position(3.5f, 3.5f);
  const float w = 512, h = 512, u = 1, v = 1;
  IndexEntry boxIndices[] = {0, 1, 2, 2, 1, 3};
  VertexPosUV vertices[] = {
    {Vec3(Position.x, Position.y, 0), Vec2(0, 0)},
    {Vec3(Position.x + w, Position.y, 0), Vec2(u, 0)},
    {Vec3(Position.x, Position.y + h, 0), Vec2(0, v)},
    {Vec3(Position.x + w, Position.y + h, 0), Vec2(u, v)},
  };

  shared_ptr<Mesh> boxMesh = make_shared<Mesh>();
  boxMesh->SetIndices(boxIndices);
  boxMesh->SetVertices(vertices);
  mMesh->Set(boxMesh);

  mDrawable->mMaterial.Connect(mMaterial);
  mDrawable->mMesh.Connect(mMesh);

  mTheScene->mDrawables.Connect(mDrawable);

  mTheScene->mCamera.GetNode()->mOrthonormal = true;
}
