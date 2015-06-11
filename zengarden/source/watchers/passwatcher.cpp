#include "passwatcher.h"

PassWatcher::PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget)
	: Watcher(PassNode, WatchWidget)
{
	GLWidget* glWidget = WatchWidget->TheGLWidget;

	glWidget->OnPaint += Delegate(this, &PassWatcher::Paint);

	TheMaterial = new Material();
	TheMaterial->mSolidPass.Connect(PassNode);

	/// Mesh
	Vec2 TopLeft(10, 10), Size(100, 100);
	IndexEntry boxIndices[] = { 0, 1, 2, 2, 1, 3 };
	VertexPos vertices[] = {
		{ Vec3(TopLeft.x, TopLeft.y, 0) },
		{ Vec3(TopLeft.x + Size.x, TopLeft.y, 0) },
		{ Vec3(TopLeft.x, TopLeft.y + Size.y, 0) },
		{ Vec3(TopLeft.x + Size.x, TopLeft.y + Size.y, 0) },
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
		if (S == &pass->mFragmentSource || S == &pass->mVertexSource) {
			GLWidget* glWidget = static_cast<GLWatcherWidget*>(mWatcherWidget)->TheGLWidget;
			glWidget->updateGL();
		}
		break;
	}
}
