#include "passwatcher.h"

PassWatcher::PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget)
  : Watcher(PassNode, WatchWidget) {
  GetGLWidget()->OnPaint += Delegate(this, &PassWatcher::Paint);

  mMaterial = new Material();
  mMaterial->mSolidPass.Connect(PassNode);

  /// Mesh
  Vec2 Position(10, 10);
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
  SafeDelete(mDrawable);
  SafeDelete(mMaterial);
}

void PassWatcher::Paint(GLWidget* Widget) {
  TheDrawingAPI->Clear(true, true, 0x80a080a0);

  Vec2 size = Vec2(Widget->width(), Widget->height());
  mGlobals.RenderTargetSize = size;
  mGlobals.RenderTargetSizeRecip = Vec2(1.0f / size.x, 1.0f / size.y);

  mGlobals.View.LoadIdentity();
  mGlobals.Projection = Matrix::Ortho(0, 0, size.x, size.y);
  mGlobals.Transformation = mGlobals.View * mGlobals.Projection;

  mDrawable->Draw(&mGlobals);
}

void PassWatcher::HandleSniffedMessage(Slot* slot, NodeMessage message,
                                       void* payload) {
  if (message == NodeMessage::NEEDS_REDRAW) {
    GetGLWidget()->updateGL();
  }
}
