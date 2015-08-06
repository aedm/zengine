#include "generalscenewatcher.h"
#include "../util/util.h"

Material* GeneralSceneWatcher::mDefaultMaterial = nullptr;

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

void GeneralSceneWatcher::Init()
{
  StubNode* defaultVertex = Util::LoadStub("engine/scenewatcher/defaultvertex.shader");
  StubNode* defaultFragment = Util::LoadStub("engine/scenewatcher/defaultfragment.shader");
  
  Pass* defaultPass = new Pass();
  defaultPass->mFragmentStub.Connect(defaultFragment);
  defaultPass->mVertexStub.Connect(defaultVertex);

  mDefaultMaterial = new Material();
  mDefaultMaterial->mSolidPass.Connect(defaultPass);
}

