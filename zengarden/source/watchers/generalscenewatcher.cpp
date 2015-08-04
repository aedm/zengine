#include "generalscenewatcher.h"

GeneralSceneWatcher::GeneralSceneWatcher(Node* node, GLWatcherWidget* watcherWidget) 
  : Watcher(node, watcherWidget)
{
  GetGLWidget()->OnPaint += Delegate(this, &GeneralSceneWatcher::Paint);

}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  SafeDelete(mDrawable);
}

void GeneralSceneWatcher::Paint(GLWidget* widget) {
  ASSERT(mDrawable);

  TheDrawingAPI->Clear(true, true, 0x202020);

  Vec2 size = Vec2(widget->width(), widget->height());
  mGlobals.RenderTargetSize = size;
  mGlobals.RenderTargetSizeRecip = Vec2(1.0f / size.x, 1.0f / size.y);

  mGlobals.View.LoadIdentity();
  mGlobals.Projection = Matrix::Ortho(0, 0, size.x, size.y);
  mGlobals.Transformation = mGlobals.View * mGlobals.Projection;

  mDrawable->Draw(&mGlobals);
}

void GeneralSceneWatcher::HandleSniffedMessage(NodeMessage message, Slot* slot, 
                                               void* payload) 
{
  if (message == NodeMessage::NEEDS_REDRAW) {
    GetGLWidget()->updateGL();
  }
}

