#include "generalscenewatcher.h"
#include "../util/util.h"
#include "../zengarden.h"

shared_ptr<Material> GeneralSceneWatcher::mDefaultMaterial;

GeneralSceneWatcher::GeneralSceneWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node) {
  if (!IsPointerOf<SceneNode>(node)) {
    mDefaultScene->mSkyLightDirection.SetDefaultValue(Vec3(0.5f, 1.0, 0.5f).Normal());
    mDefaultScene->mSkyLightColor.SetDefaultValue(Vec3(1.0f, 1.0f, 1.0f));
    mDefaultScene->mSkyLightAmbient.SetDefaultValue(0.2f);
    mDefaultScene->mCamera.Connect(mCamera);
    mScene = mDefaultScene;
    mRenderForwarder = mDefaultScene->Watch<RenderForwarder>(mDefaultScene);
    mRenderForwarder->mOnRedraw = Delegate(this, &GeneralSceneWatcher::OnRedraw);
  }
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  if (mRenderForwarder) mRenderForwarder->mOnRedraw.clear();
}

void GeneralSceneWatcher::Paint(EventForwarderGLWidget* widget) {
  if (!mWatcherWidget) return;
  shared_ptr<SceneNode> sceneNode = PointerCast<SceneNode>(mScene->GetReferencedNode());
  sceneNode->Update();
  sceneNode->UpdateDependencies();

  /// Nvidia driver will fail to bind framebuffers after shader compilation.
  /// Just skip the frame if a shader was edited/updated.
  /// TODO: revisit this every now and then.
  if (OpenGL->mProgramCompiledHack) {
    OpenGL->mProgramCompiledHack = false;
    return;
  }

  if (!mRenderTarget) {
    mRenderTarget = new RenderTarget(Vec2(float(mWatcherWidget->width()),
                                          float(mWatcherWidget->height())));
  }

  Vec2 size = Vec2(widget->width(), widget->height());
  mRenderTarget->Resize(size);

  mRenderTarget->SetGBufferAsTarget(&mGlobals);
  OpenGL->Clear(true, true, 0x303030);

  sceneNode->Draw(mRenderTarget, &mGlobals);

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
  GetGLWidget()->OnMouseRelease += 
    Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  GetGLWidget()->OnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);
}

void GeneralSceneWatcher::Init() {
  shared_ptr<StubNode> defaultVertex = 
    Util::LoadStub("engine/scenewatcher/defaultvertex.shader");
  shared_ptr<StubNode> defaultFragment =
    Util::LoadStub("engine/scenewatcher/defaultfragment.shader");

  shared_ptr<Pass> defaultPass = make_shared<Pass>();
  defaultPass->mFragmentStub.Connect(defaultFragment);
  defaultPass->mVertexStub.Connect(defaultVertex);
  defaultPass->mRenderstate.mDepthTest = true;
  defaultPass->mFaceModeSlot.SetDefaultValue(0);
  defaultPass->mBlendModeSlot.SetDefaultValue(0);

  mDefaultMaterial = make_shared<Material>();
  mDefaultMaterial->mSolidPass.Connect(defaultPass);
}

void GeneralSceneWatcher::HandleMousePress(EventForwarderGLWidget*, QMouseEvent* event) {
  mOriginalPosition = event->pos();
  mOriginalOrientation = mCamera->mOrientation.Get();
  mOriginalDistance = mCamera->mDistance.Get();
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
    Vec3 orientation = mCamera->mOrientation.Get();
    orientation.y = mOriginalOrientation.y + float(diff.x()) / 300.0f;
    orientation.x = mOriginalOrientation.x + float(diff.y()) / 300.0f;
    mCamera->mOrientation.SetDefaultValue(orientation);
  } else if (event->buttons() & Qt::RightButton) {
    auto diff = event->pos() - mOriginalPosition;
    float distance = mCamera->mDistance.Get();
    distance = mOriginalDistance - float(diff.y()) / 2.0f;
    mCamera->mDistance.SetDefaultValue(distance);
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

RenderForwarder::RenderForwarder(const shared_ptr<SceneNode>& node)
  : Watcher(node) {}

void RenderForwarder::OnRedraw() {
  if (!mOnRedraw.empty()) mOnRedraw();
}

