#include "passwatcher.h"

PassWatcher::PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget)
	: Watcher(PassNode, WatchWidget)
{
	GLWidget* glWidget = WatchWidget->TheGLWidget;

	glWidget->OnPaint += Delegate(this, &PassWatcher::Paint);

	TheMaterial = new Material();
	TheMaterial->mSolidPass.Connect(PassNode);

	/// Mesh
	Vec2 Position(10, 10);
  float w = 512, h = 512, u = 1, v = 1;
	IndexEntry boxIndices[] = { 0, 1, 2, 2, 1, 3 };
  VertexPosUV vertices[] = {
    {Vec3(Position.x, Position.y, 0), Vec2(0, 0)},
    {Vec3(Position.x + w, Position.y, 0), Vec2(u, 0)},
    {Vec3(Position.x, Position.y + h, 0), Vec2(0, v)},
    {Vec3(Position.x + w, Position.y + h, 0), Vec2(u, v)},
  };

	Mesh* boxMesh = TheResourceManager->CreateMesh();
	boxMesh->SetIndices(boxIndices);
	boxMesh->SetVertices(vertices);
	TheMesh = StaticMeshNode::Create(boxMesh);

	TheDrawable = new Drawable();
	TheDrawable->mMaterial.Connect(TheMaterial);
	TheDrawable->mMesh.Connect(TheMesh);
}

PassWatcher::~PassWatcher() 
{
	SafeDelete(TheDrawable);
	SafeDelete(TheMaterial);
}

void PassWatcher::Paint(GLWidget* Widget)
{
	TheDrawingAPI->Clear(true, true, 0x80a080a0);

	Vec2 size = Vec2(Widget->width(), Widget->height());
	TheGlobals.RenderTargetSize = size;
	TheGlobals.RenderTargetSizeRecip = Vec2(1.0f / size.x, 1.0f / size.y);

	TheGlobals.View.LoadIdentity();
	TheGlobals.Projection = Matrix::Ortho(0, 0, size.x, size.y);
	TheGlobals.Transformation = TheGlobals.View * TheGlobals.Projection;

	TheDrawable->Draw(&TheGlobals);
}

void PassWatcher::HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::NEEDS_REDRAW:
	case NodeMessage::VALUE_CHANGED:
		Pass* pass = static_cast<Pass*>(GetNode());
		if (S == pass->GetFragmentSourceSlot() || S == pass->GetVertexSourceSlot()) {
			GLWidget* glWidget = static_cast<GLWatcherWidget*>(mWatcherWidget)->TheGLWidget;
			glWidget->updateGL();
		}
		break;
	}
}
