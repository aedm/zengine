#include "passwatcher.h"

PassWatcher::PassWatcher(Pass* PassNode, GLWatcherWidget* WatchWidget)
	: Watcher(PassNode, WatchWidget)
{
	GLWidget* glWidget = WatchWidget->TheGLWidget;

	glWidget->OnPaint += Delegate(this, &PassWatcher::Paint);

	TheMaterial = new Material();
	TheMaterial->SolidPass.Connect(PassNode);

	TheMesh = new StaticMeshNode();
	

	TheDrawable = new Drawable();
	TheDrawable->TheMaterial.Connect(TheMaterial);
	TheDrawable->TheMesh.Connect(TheMesh);
}

PassWatcher::~PassWatcher() 
{
	SafeDelete(TheDrawable);
	SafeDelete(TheMaterial);
}

void PassWatcher::Paint(GLWidget* Widget)
{
	TheDrawingAPI->Clear(true, true, 0xff00ff00);

	TheDrawable->Draw(&TheGlobals);
}
