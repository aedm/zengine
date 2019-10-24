#include <include/nodes/scenenode.h>
#include <include/shaders/engineshaders.h>
REGISTER_NODECLASS(SceneNode, "Scene");

SceneNode::SceneNode()
  : mDrawables(this, "Drawables", true)
  , mCamera(this, "Camera")
  , mShadowMapSize(this, "Shadow map size", false, true, true, 0.0f, 100.0f)
  , mSkyLightDirection(this, "Skylight direction")
  , mSkyLightColor(this, "Skylight color")
  , mSkyLightAmbient(this, "Ambient factor")
  , mSkyLightSpread(this, "Shadow spread", false, true, true, 0.0f, 30.0f)
  , mSkyLightSampleSpread(this, "Sample spread", false, true, true, 0.0f, 20.0f)
  , mSkyLightBias(this, "Shadow bias")
  , mDOFEnabled(this, "DOF enabled")
  , mDOFFocusDistance(this, "DOF focus distance", false, true, true, 0.0f, 20.0f)
  , mDOFBlur(this, "DOF blur", false, true, true, 0.0f, 30.0f)
  , mDOFScale(this, "DOF scale", false, true, true, 0.0f, 10.0f)
  , mDOFBleed(this, "DOF bleed", false, true, true, 0.0f, 0.1f)
  , mZPrepassDisabled(this, "Disable Z prepass")
  , mFluidsSlot(this, "Fluids", true)
  , mSceneTimes(this, std::string(), true, false, false, false)
{
  mSkyLightSpread.SetDefaultValue(10.0f);
  mSkyLightSampleSpread.SetDefaultValue(5.0f);
  mShadowMapSize.SetDefaultValue(Vec3(30, 30, 50));
  mSkyLightDirection.SetDefaultValue(Vec3(0.5f, 0.5f, 0.5f));
  mSkyLightColor.SetDefaultValue(Vec3(1, 1, 1));
  mSkyLightAmbient.SetDefaultValue(0.2f);
  mDOFEnabled.SetDefaultValue(0.0f);
  mDOFFocusDistance.SetDefaultValue(50.0f);
  mDOFBlur.SetDefaultValue(10.0f);
  mDOFScale.SetDefaultValue(3.0f);
  mDOFBleed.SetDefaultValue(0.04f);
}

SceneNode::~SceneNode() = default;

void SceneNode::Draw(RenderTarget* renderTarget, Globals* globals) {
  const float globalTime = mGlobalTimeNode->Get();
  const float passedTime = globalTime - mLastRenderTime;
  mLastRenderTime = globalTime;

  const float fluidAdvanceTime = passedTime > 0.1f ? 0.1f : passedTime;

  /// Paint Fluids
  RenderDrawables(globals, PassType::FLUID_PAINT);

  /// Simulate Fluids
  for (UINT i = 0; i < mFluidsSlot.GetMultiNodeCount(); i++) {
    auto& fluid = PointerCast<FluidNode>(mFluidsSlot.GetReferencedMultiNode(i));
    fluid->Render(fluidAdvanceTime);
  }

  const bool directToScreen = globals->DirectToScreen > 0.5f;
  const bool directToSquare = globals->DirectToSquare > 0.5f;

  /// Get camera
  const auto& camera = mCamera.GetNode();
  if (camera == nullptr) return;

  /// Pass #1: skylight shadow
  renderTarget->SetShadowBufferAsTarget(globals);
  OpenGL->Clear(true, true, 0xff00ff80);
  const Vec3 s = mShadowMapSize.Get();
  const Vec3 lightDir = mSkyLightDirection.Get().Normal();

  const Matrix lookAt = Matrix::LookAt(-lightDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
  const Matrix target = Matrix::Translate(-camera->mTarget.Get());
  globals->Camera = lookAt * target;
 
  globals->Projection = Matrix::Ortho(-s.x, -s.y, s.x, s.y, -s.z, s.z);
  globals->World.LoadIdentity();
    /// Calculate shadow center
  Vec3 shadowCenter(0, 0, 0);
  for (UINT i = 0; i < mDrawables.GetMultiNodeCount(); i++) {
    auto& drawable = PointerCast<Drawable>(mDrawables.GetReferencedMultiNode(i));
    drawable->ComputeForcedShadowCenter(globals, shadowCenter);
  }
  const Matrix shadowCenterTarget = Matrix::Translate(-shadowCenter);
  globals->Camera = shadowCenterTarget * globals->Camera;

  globals->SkylightProjection = globals->Projection;
  globals->SkylightCamera = globals->Camera;
  globals->SkylightDirection = mSkyLightDirection.Get();
  globals->SkylightColor = mSkyLightColor.Get();
  globals->SkylightAmbient = mSkyLightAmbient.Get();
  globals->SkylightSpread = mSkyLightSpread.Get();
  globals->SkylightSampleSpread = mSkyLightSampleSpread.Get();
  globals->SkylightBias = mSkyLightBias.Get();
  globals->SkylightTexture = nullptr;
  globals->Time = globalTime;
  RenderDrawables(globals, PassType::SHADOW);

  /// Set up render target
  if (directToSquare) {
    renderTarget->SetSquareBufferAsTarget(globals);
  }
  else if (directToScreen) {
    renderTarget->SetColorBufferAsTarget(globals);
  }
  else {
    renderTarget->SetGBufferAsTarget(globals);
  }

  globals->SkylightBias = 0;

  /// Pass #2: Z prepass, using the shadow pass material
  if (mZPrepassDisabled.Get() < 0.5) {
    globals->SkylightTexture = renderTarget->mShadowTexture;

    camera->SetupGlobals(globals);
    RenderDrawables(globals, PassType::SHADOW);
  }

  /// Pass #3: draw to G-Buffer / screen
  camera->SetupGlobals(globals);
  globals->SkylightTexture = renderTarget->mShadowTexture;
  RenderDrawables(globals, PassType::SOLID);

  /// Hack: set DOF settings
  if (!directToSquare && !directToScreen) {
    /// Pass #4: Z Postpass
    camera->SetupGlobals(globals);
    renderTarget->SetGBufferAsTargetForZPostPass(globals);
    globals->SkylightTexture = renderTarget->mShadowTexture;
    RenderDrawables(globals, PassType::ZPOST);

    globals->PPDofEnabled = mDOFEnabled.Get();
    globals->PPDofFocusDistance = mDOFFocusDistance.Get();
    globals->PPDofBlur = mDOFBlur.Get();
    globals->PPDofScale = mDOFScale.Get();
    globals->PPDofBleed= mDOFBleed.Get();
  }
}

void SceneNode::Operate() {
  CalculateRenderDependencies();
}

void SceneNode::RenderDrawables(Globals* globals, PassType passType) const
{
  for (UINT i = 0; i < mDrawables.GetMultiNodeCount(); i++) {
    auto& drawable = PointerCast<Drawable>(mDrawables.GetReferencedMultiNode(i));
    drawable->Draw(globals, passType);
  }
}

void SceneNode::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::TRANSITIVE_CLOSURE_CHANGED:
    mIsUpToDate = false;
    break;
  case MessageType::VALUE_CHANGED:
  case MessageType::SLOT_CONNECTION_CHANGED:
    EnqueueMessage(MessageType::NEEDS_REDRAW);
    break;
  case MessageType::SCENE_TIME_EDITED:
  {
    auto source = PointerCast<SceneTimeNode>(message->mSource);
    mSceneTime = source->Get();
    for (auto& node : mSceneTimes.GetDirectMultiNodes()) {
      if (node != source) {
        PointerCast<SceneTimeNode>(node)->Set(mSceneTime);
      }
    }
    SendMsg(MessageType::SCENE_TIME_EDITED);
    break;
  }
  default: break;
  }
}

void SceneNode::CalculateRenderDependencies() {
  mTransitiveClosure.clear();
  mSceneTimes.DisconnectAll(false);
  GenerateTransitiveClosure(mTransitiveClosure, true);
  for (auto& node : mTransitiveClosure) {
    if (IsExactType<SceneTimeNode>(node)) {
      std::shared_ptr<SceneTimeNode> sceneTimeNode = PointerCast<SceneTimeNode>(node);
      ASSERT(sceneTimeNode);
      mSceneTimes.Connect(sceneTimeNode);
    }
  }
}

void SceneNode::SetSceneTime(float beats) {
  Update();
  for (auto& node : mSceneTimes.GetDirectMultiNodes()) {
    PointerCast<SceneTimeNode>(node)->Set(beats);
  }
}

float SceneNode::GetSceneTime() const {
  return mSceneTime;
}

void SceneNode::UpdateDependencies() {
  /// Update dependencies
  for (auto& node : mTransitiveClosure) {
    if (this != node.get()) node->Update();
  }
}
