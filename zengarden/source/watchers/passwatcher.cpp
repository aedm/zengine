#include "passwatcher.h"

PassWatcher::PassWatcher(Pass* pass, GLWatcherWidget* watcherWidget)
  : GeneralSceneWatcher(pass, watcherWidget) 
{
  mMaterial = new Material();
  mMaterial->mSolidPass.Connect(pass);

  /// Mesh
  Vec2 Position(3.5f, 3.5f);
  float w = 512, h = 512, u = 1, v = 1;
  IndexEntry boxIndices[] = {0, 1, 2, 2, 1, 3};
  VertexPosUV vertices[] = {
    {Vec3(Position.x, Position.y, 0), Vec2(0, 0)},
    {Vec3(Position.x + w, Position.y, 0), Vec2(u, 0)},
    {Vec3(Position.x, Position.y + h, 0), Vec2(0, v)},
    {Vec3(Position.x + w, Position.y + h, 0), Vec2(u, v)},
  };

  Mesh* boxMesh = TheResourceManager->CreateMesh();
  boxMesh->SetIndices(boxIndices);
  boxMesh->SetVertices(vertices);
  mMesh = StaticMeshNode::Create(boxMesh);

  mDrawable = new Drawable();
  mDrawable->mMaterial.Connect(mMaterial);
  mDrawable->mMesh.Connect(mMesh);
}

PassWatcher::~PassWatcher() {
  SafeDelete(mMaterial);
  SafeDelete(mMesh);
}
