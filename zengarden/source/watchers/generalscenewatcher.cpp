#include "generalscenewatcher.h"
#include "../util/util.h"

Material* GeneralSceneWatcher::mDefaultMaterial = nullptr;

GeneralSceneWatcher::GeneralSceneWatcher(Node* node, GLWatcherWidget* watcherWidget) 
  : Watcher(node, watcherWidget)
{
  GetGLWidget()->OnPaint += Delegate(this, &GeneralSceneWatcher::Paint);
  GetGLWidget()->OnMousePress += Delegate(this, &GeneralSceneWatcher::HandleMousePress);
  GetGLWidget()->OnMouseRelease += Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  GetGLWidget()->OnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);

  mDefaultScene.mCamera.Connect(&mCamera);
  mScene = &mDefaultScene;
  mDefaultScene.onSniffMessage += Delegate(this, &GeneralSceneWatcher::SniffMessage);
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  mDefaultScene.onSniffMessage -= Delegate(this, &GeneralSceneWatcher::SniffMessage);
  SafeDelete(mDrawable);
}

void GeneralSceneWatcher::Paint(GLWidget* widget) {
  TheDrawingAPI->Clear(true, true, 0x303030);

  Vec2 size = Vec2(widget->width(), widget->height());
  mScene->Draw(size);
}

void GeneralSceneWatcher::HandleSniffedMessage(NodeMessage message, Slot* slot, 
                                               void* payload) 
{
  if (message == NodeMessage::NEEDS_REDRAW) {
    GetGLWidget()->update();
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

void GeneralSceneWatcher::HandleMousePress(GLWidget*, QMouseEvent* event) {
  mOriginalPosition = event->pos();
  mOriginalOrientation = mCamera.mOrientation.Get();
  mOriginalDistance = mCamera.mDistance.Get();
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void GeneralSceneWatcher::HandleMouseRelease(GLWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}

void GeneralSceneWatcher::HandleMouseMove(GLWidget*, QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    auto diff = event->pos() - mOriginalPosition;
    Vec3 orientation = mCamera.mOrientation.Get();
    orientation.y = mOriginalOrientation.y - float(diff.x()) / 300.0f;
    orientation.x = mOriginalOrientation.x - float(diff.y()) / 300.0f;
    mCamera.mOrientation.SetDefaultValue(orientation);
  }
  else if (event->buttons() & Qt::RightButton) {
    auto diff = event->pos() - mOriginalPosition;
    float distance = mCamera.mDistance.Get();
    distance = mOriginalDistance - float(diff.y()) / 2.0f;
    mCamera.mDistance.SetDefaultValue(distance);
  }
}

void GeneralSceneWatcher::HandleMouseWheel(GLWidget*, QWheelEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleKeyPress(GLWidget*, QKeyEvent* event) {

}

