#include "generalscenewatcher.h"
#include "../util/util.h"
#include "../zengarden.h"

Material* GeneralSceneWatcher::mDefaultMaterial = nullptr;

GeneralSceneWatcher::GeneralSceneWatcher(Node* node) 
  : WatcherUI(node)
{
  if (node->GetType() != NodeType::SCENE) {
    mDefaultScene.mSkyLightDirection.SetDefaultValue(Vec3(0.5f, 1.0, 0.5f).Normal());
    mDefaultScene.mSkyLightColor.SetDefaultValue(Vec3(1.0f, 1.0f, 1.0f));
    mDefaultScene.mSkyLightAmbient.SetDefaultValue(0.2f);
    mDefaultScene.mCamera.Connect(&mCamera);
    mScene = &mDefaultScene;
    mRenderForwarder = mDefaultScene.Watch<RenderForwarder>(&mDefaultScene);
    mRenderForwarder->mOnRedraw = Delegate(this, &GeneralSceneWatcher::OnRedraw);
  }
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  mRenderForwarder = nullptr;
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

  mRenderTarget->SetGBufferAsTarget(&mGlobals);
  OpenGL->Clear(true, true, 0x303030);

  mScene->Draw(mRenderTarget, &mGlobals);

  /// Apply post-process to scene to framebuffer
  TheEngineShaders->ApplyPostProcess(mRenderTarget, &mGlobals);
}

void GeneralSceneWatcher::OnRedraw() {
  if (mWatcherWidget) GetGLWidget()->update();
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
  defaultPass->mRenderstate.mDepthTest = true;
  defaultPass->mFaceModeSlot.SetDefaultValue(0);
  defaultPass->mBlendModeSlot.SetDefaultValue(0);

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
    orientation.y = mOriginalOrientation.y + float(diff.x()) / 300.0f;
    orientation.x = mOriginalOrientation.x + float(diff.y()) / 300.0f;
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

