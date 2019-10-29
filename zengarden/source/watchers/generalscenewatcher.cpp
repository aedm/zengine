#include "generalscenewatcher.h"
#include "../util/util.h"
#include "../zengarden.h"
#include <memory>

std::shared_ptr<Material> GeneralSceneWatcher::mDefaultMaterial;

GeneralSceneWatcher::GeneralSceneWatcher(const std::shared_ptr<Node>& node)
  : WatcherUi(node) {
  if (IsPointerOf<SceneNode>(node)) {
    mTheScene = PointerCast<SceneNode>(node);
    return;
  }
  auto sceneNode = std::make_shared<SceneNode>();
  sceneNode->mSkyLightDirection.SetDefaultValue(normalize(vec3(0.5f, 1.0, 0.5f)));
  sceneNode->mSkyLightColor.SetDefaultValue(vec3(1.0f, 1.0f, 1.0f));
  sceneNode->mSkyLightAmbient.SetDefaultValue(0.2f);
  sceneNode->mCamera.Connect(std::make_shared<CameraNode>());
  mRenderForwarder = sceneNode->Watch<RenderForwarder>(sceneNode);
  mRenderForwarder->mOnRedraw = Delegate(this, &GeneralSceneWatcher::OnRedraw);
  mTheScene = sceneNode;
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  if (mRenderForwarder) mRenderForwarder->mOnRedraw.clear();
}

void GeneralSceneWatcher::Paint(EventForwarderGlWidget* widget) {
  if (!mWatcherWidget) return;
  //std::shared_ptr<SceneNode> sceneNode = PointerCast<SceneNode>(mScene->GetReferencedNode());
  mTheScene->Update();
  mTheScene->UpdateDependencies();

  /// Nvidia driver will fail to bind framebuffers after shader compilation.
  /// Just skip the frame if a shader was edited/updated.
  /// TODO: revisit this every now and then.
  if (OpenGL->mProgramCompiledHack) {
    OpenGL->mProgramCompiledHack = false;
    return;
  }

  if (!mRenderTarget) {
    mRenderTarget = new RenderTarget(vec2(float(mWatcherWidget->width()),
                                          float(mWatcherWidget->height())));
  }

  const vec2 size = vec2(widget->width(), widget->height());
  mRenderTarget->Resize(size);

  mRenderTarget->SetGBufferAsTarget(&mGlobals);
  OpenGL->Clear(true, true, 0x303030);

  mTheScene->Draw(mRenderTarget, &mGlobals);

  /// Apply post-process to scene to framebuffer
  TheEngineShaders->ApplyPostProcess(mRenderTarget, &mGlobals);
}

void GeneralSceneWatcher::OnRedraw() {
  if (mWatcherWidget) GetGlWidget()->update();
}

void GeneralSceneWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);
  watcherWidget->GetGLWidget()->setFocusPolicy(Qt::ClickFocus);

  GetGlWidget()->mOnPaint += Delegate(this, &GeneralSceneWatcher::Paint);
  GetGlWidget()->mOnMousePress += Delegate(this, &GeneralSceneWatcher::HandleMousePress);
  GetGlWidget()->mOnMouseRelease += 
    Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  GetGlWidget()->mOnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  GetGlWidget()->mOnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  GetGlWidget()->mOnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);
}

void GeneralSceneWatcher::Init() {
  const std::shared_ptr<StubNode> defaultVertex = 
    Util::LoadStub("engine/scenewatcher/defaultvertex.shader");
  const std::shared_ptr<StubNode> defaultFragment =
    Util::LoadStub("engine/scenewatcher/defaultfragment.shader");

  std::shared_ptr<Pass> defaultPass = std::make_shared<Pass>();
  defaultPass->mFragmentStub.Connect(defaultFragment);
  defaultPass->mVertexStub.Connect(defaultVertex);
  defaultPass->mRenderstate.mDepthTest = true;
  defaultPass->mFaceModeSlot.SetDefaultValue(0);
  defaultPass->mBlendModeSlot.SetDefaultValue(0);

  mDefaultMaterial = std::make_shared<Material>();
  mDefaultMaterial->mSolidPass.Connect(defaultPass);
}

void GeneralSceneWatcher::HandleMousePress(EventForwarderGlWidget*, QMouseEvent* event) {
  const std::shared_ptr<CameraNode> camera = mTheScene->mCamera.GetNode();
  if (!camera) return;

  mOriginalPosition = event->pos();
  mOriginalOrientation = camera->mOrientation.Get();
  mOriginalDistance = camera->mDistance.Get();
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void GeneralSceneWatcher::HandleMouseRelease(EventForwarderGlWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}

void GeneralSceneWatcher::HandleMouseMove(EventForwarderGlWidget*, QMouseEvent* event) const
{
  std::shared_ptr<CameraNode> camera = mTheScene->mCamera.GetNode();
  if (!camera) return;

  if (event->buttons() & Qt::LeftButton) {
    const auto diff = event->pos() - mOriginalPosition;
    vec3 orientation = camera->mOrientation.Get();
    const float dx = float(diff.x()) / 300.0f;
    const float dy = float(diff.y()) / 300.0f;
    orientation.y = mOriginalOrientation.y + dx;
    orientation.x = mOriginalOrientation.x + dy;
    if (camera->mOrientation.IsDefaulted()) {
      camera->mOrientation.SetDefaultValue(orientation);
      return;
    }
    const auto node = camera->mOrientation.GetReferencedNode();
    if (IsPointerOf<FloatsToVec3Node>(node)) {
      auto floatsNode = PointerCast<FloatsToVec3Node>(node);
      auto& xSlot = floatsNode->mX;
      auto& ySlot = floatsNode->mY;
      xSlot.SetDefaultValue(orientation.x);
      ySlot.SetDefaultValue(orientation.y);
      /// If it's connected to a spline, set base offset
      if (!xSlot.IsDefaulted() && IsPointerOf<FloatSplineNode>(xSlot.GetNode())) {
        PointerCast<FloatSplineNode>(xSlot.GetNode())->SetBaseOffset(dy);
      }
      if (!ySlot.IsDefaulted() && IsPointerOf<FloatSplineNode>(ySlot.GetNode())) {
        PointerCast<FloatSplineNode>(ySlot.GetNode())->SetBaseOffset(dx);
      }
    }
  } else if (event->buttons() & Qt::RightButton) {
    const auto diff = event->pos() - mOriginalPosition;
    const float distance = mOriginalDistance - float(diff.y()) / 2.0f;
    camera->mDistance.SetDefaultValue(distance);
  }
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void GeneralSceneWatcher::HandleMouseWheel(EventForwarderGlWidget*, QWheelEvent* event) {

}

// ReSharper disable once CppMemberFunctionMayBeStatic
void GeneralSceneWatcher::HandleMouseLeftDown(QMouseEvent* event) {

}

// ReSharper disable once CppMemberFunctionMayBeStatic
void GeneralSceneWatcher::HandleMouseLeftUp(QMouseEvent* event) {

}

// ReSharper disable once CppMemberFunctionMayBeStatic
void GeneralSceneWatcher::HandleMouseRightDown(QMouseEvent* event) {

}

// ReSharper disable once CppMemberFunctionMayBeStatic
void GeneralSceneWatcher::HandleMouseRightUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleKeyPress(EventForwarderGlWidget*, QKeyEvent* event) const
{
  if (event->key() == Qt::Key_S) {
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(mTheScene);
  }
  if (event->key() == Qt::Key_C) {
    const auto camera = mTheScene->mCamera.GetNode();
    if (!camera) return;
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(camera);
  }
  if (event->key() == Qt::Key_Return) {
    /// Bake values to camera splines
    const std::shared_ptr<CameraNode> camera = mTheScene->mCamera.GetNode();
    if (!camera) return;
    const auto node = camera->mOrientation.GetReferencedNode();
    if (IsPointerOf<FloatsToVec3Node>(node)) {
      auto floatsNode = PointerCast<FloatsToVec3Node>(node);
      auto& xSlot = floatsNode->mX;
      auto& ySlot = floatsNode->mY;
      if (!xSlot.IsDefaulted() && IsPointerOf<FloatSplineNode>(xSlot.GetNode())) {
        PointerCast<FloatSplineNode>(xSlot.GetNode())->AddBasePointWithOffset();
      }
      if (!ySlot.IsDefaulted() && IsPointerOf<FloatSplineNode>(ySlot.GetNode())) {
        PointerCast<FloatSplineNode>(ySlot.GetNode())->AddBasePointWithOffset();
      }
    }
  }
}

RenderForwarder::RenderForwarder(const std::shared_ptr<SceneNode>& node)
  : Watcher(node) {}

void RenderForwarder::OnRedraw() {
  if (!mOnRedraw.empty()) mOnRedraw();
}

