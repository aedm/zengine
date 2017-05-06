#include "generalscenewatcher.h"
#include "../util/util.h"

Material* GeneralSceneWatcher::mDefaultMaterial = nullptr;

GeneralSceneWatcher::GeneralSceneWatcher(Node* node) 
  : WatcherUI(node)
{
  if (node->GetType() != NodeType::SCENE) {
    mDefaultScene.mCamera.Connect(&mCamera);
    mScene = &mDefaultScene;
    mRenderForwarder = mDefaultScene.Watch<RenderForwarder>(&mDefaultScene);
    mRenderForwarder->mOnRedraw = Delegate(this, &GeneralSceneWatcher::OnRedraw);
  }
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  if (mRenderForwarder) mRenderForwarder->Unwatch();
  SafeDelete(mDrawable);
  SafeDelete(mRenderTarget);
}

void GeneralSceneWatcher::Paint(EventForwarderGLWidget* widget) {
  if (!mWatcherWidget) return;
  if (!mRenderTarget) {
    GetGLWidget()->makeCurrent();
    mRenderTarget =
      new RenderTarget(Vec2(float(mWatcherWidget->width()), float(mWatcherWidget->height())));
  }

  Vec2 size = Vec2(widget->width(), widget->height());
  mRenderTarget->Resize(size);
  mScene->Draw(mRenderTarget);
}

void GeneralSceneWatcher::OnRedraw() {
  GetGLWidget()->update();
}

void GeneralSceneWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);

  GetGLWidget()->OnPaint += Delegate(this, &GeneralSceneWatcher::Paint);
  GetGLWidget()->OnMousePress += Delegate(this, &GeneralSceneWatcher::HandleMousePress);
  GetGLWidget()->OnMouseRelease += Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  GetGLWidget()->OnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);
}

void GeneralSceneWatcher::Init()
{
  StubNode* defaultVertex = Util::LoadStub("engine/scenewatcher/defaultvertex.shader");
  StubNode* defaultFragment = Util::LoadStub("engine/scenewatcher/defaultfragment.shader");
  
  Pass* defaultPass = new Pass();
  defaultPass->mFragmentStub.Connect(defaultFragment);
  defaultPass->mVertexStub.Connect(defaultVertex);
  defaultPass->mRenderstate.DepthTest = true;
  defaultPass->mRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;
  defaultPass->mRenderstate.BlendMode = RenderState::BLEND_ALPHA;

  mDefaultMaterial = new Material();
  mDefaultMaterial->mSolidPass.Connect(defaultPass);
}

void GeneralSceneWatcher::HandleMousePress(EventForwarderGLWidget*, QMouseEvent* event) {
  mOriginalPosition = event->pos();
  mOriginalOrientation = mCamera.mOrientation.Get();
  mOriginalDistance = mCamera.mDistance.Get();
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void GeneralSceneWatcher::HandleMouseRelease(EventForwarderGLWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}

void GeneralSceneWatcher::HandleMouseMove(EventForwarderGLWidget*, QMouseEvent* event) {
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

void GeneralSceneWatcher::HandleMouseWheel(EventForwarderGLWidget*, QWheelEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleKeyPress(EventForwarderGLWidget*, QKeyEvent* event) {

}

RenderForwarder::RenderForwarder(SceneNode* node)
  : Watcher(node)
{}

void RenderForwarder::OnRedraw() {
  mOnRedraw();
}

