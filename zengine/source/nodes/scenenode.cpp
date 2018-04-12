#include <include/nodes/scenenode.h>
#include <include/shaders/engineshaders.h>
REGISTER_NODECLASS(SceneNode, "Scene");

static SharedString DrawablesSlotName = make_shared<string>("Drawables");
static SharedString CameraSlotName = make_shared<string>("Camera");
static SharedString ShadowMapSizeSlotName = make_shared<string>("Shadow map size");
static SharedString SkylightDirectionSlotName = make_shared<string>("Skylight direction");
static SharedString SkylightColorSlotName = make_shared<string>("Skylight color");
static SharedString SkylightAmbientSlotName = make_shared<string>("Ambient factor");
static SharedString SkylightSpreadSlotName = make_shared<string>("Shadow spread");
static SharedString SkylightSampleSpreadSlotName = make_shared<string>("Sample spread");
static SharedString DOFEnabledSlotName = make_shared<string>("DOF enabled");
static SharedString DOFFocusDistanceSlotName = make_shared<string>("DOF focus distance");
static SharedString DOFBlurSlotName = make_shared<string>("DOF blur");

SceneNode::SceneNode()
  : mDrawables(this, DrawablesSlotName, true)
  , mCamera(this, CameraSlotName)
  , mShadowMapSize(this, ShadowMapSizeSlotName, false, true, true, 0.0f, 100.0f)
  , mSkyLightDirection(this, SkylightDirectionSlotName)
  , mSkyLightColor(this, SkylightColorSlotName)
  , mSkyLightAmbient(this, SkylightAmbientSlotName)
  , mSkyLightSpread(this, SkylightSpreadSlotName, false, true, true, 0.0f, 30.0f)
  , mSkyLightSampleSpread(this, SkylightSampleSpreadSlotName, false, true, true, 0.0f, 20.0f)
  , mSceneTimes(this, nullptr, true, false, false, false)
  , mDOFEnabled(this, DOFEnabledSlotName) 
  , mDOFFocusDistance(this, DOFFocusDistanceSlotName, false, true, true, 0.0f, 100.0f)
  , mDOFBlur(this, DOFBlurSlotName, false, true, true, 0.0f, 30.0f)
{
  mSkyLightSpread.SetDefaultValue(10.0f, true);
  mSkyLightSampleSpread.SetDefaultValue(5.0f, true);
  mShadowMapSize.SetDefaultValue(Vec3(30, 30, 50), true);
  mSkyLightDirection.SetDefaultValue(Vec3(0.5f, 0.5f, 0.5f), true);
  mSkyLightColor.SetDefaultValue(Vec3(1, 1, 1), true);
  mSkyLightAmbient.SetDefaultValue(0.2f, true);
  mDOFEnabled.SetDefaultValue(0.0f, true);
  mDOFFocusDistance.SetDefaultValue(50.0f, true);
  mDOFBlur.SetDefaultValue(10.0f, true);
}

SceneNode::~SceneNode() {
}

void SceneNode::Draw(RenderTarget* renderTarget, Globals* globals) {
  bool directToScreen = globals->DirectToScreen > 0.5f;

  /// Get camera
  auto& camera = mCamera.GetNode();
  if (camera == nullptr) return;

  /// Pass #1: skylight shadow
  renderTarget->SetShadowBufferAsTarget(globals);
  OpenGL->Clear(true, true, 0xff00ff80);

  Vec3 s = mShadowMapSize.Get();
  Vec3 lightDir = mSkyLightDirection.Get().Normal();

  //Vec3 target = camera->mTarget.Get();
  Matrix lookat = Matrix::LookAt(-lightDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
  Matrix target = Matrix::Translate(-camera->mTarget.Get());
  //Matrix distance = Matrix::Translate(Vec3(0, 0, -camera->mDistance.Get()));
  globals->Camera = lookat * target;
  //globals->Camera = Matrix::LookAt(target-lightDir, target, Vec3(0, 1, 0));

  globals->Projection = Matrix::Ortho(-s.x, -s.y, s.x, s.y, -s.z, s.z);
  globals->World.LoadIdentity();
  globals->SkylightProjection = globals->Projection;
  globals->SkylightCamera = globals->Camera;
  globals->SkylightDirection = mSkyLightDirection.Get();
  globals->SkylightColor = mSkyLightColor.Get();
  globals->SkylightAmbient = mSkyLightAmbient.Get();
  globals->SkylightSpread = mSkyLightSpread.Get();
  globals->SkylightSampleSpread = mSkyLightSampleSpread.Get();
  RenderDrawables(globals, PassType::SHADOW);

  /// Pass #2: draw to G-Buffer / screen
  if (directToScreen) {
    renderTarget->SetColorBufferAsTarget(globals);
  } else {
    renderTarget->SetGBufferAsTarget(globals);
  }
  //OpenGL->Clear(true, true, 0x303030);
  camera->SetupGlobals(globals);
  globals->SkylightTexture = renderTarget->mShadowTexture;
  RenderDrawables(globals, PassType::SOLID);

  /// Hack: set DOF settings
  if (!directToScreen) {
    globals->PPDofEnabled = mDOFEnabled.Get();
    globals->PPDofFocusDistance = mDOFFocusDistance.Get();
    globals->PPDofBlur = mDOFBlur.Get();
  }
}

void SceneNode::Operate() {
  CalculateRenderDependencies();
}

void SceneNode::RenderDrawables(Globals* globals, PassType passType) {
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
      shared_ptr<SceneTimeNode> sceneTimeNode = PointerCast<SceneTimeNode>(node);
      ASSERT(sceneTimeNode);
      mSceneTimes.Connect(sceneTimeNode);
    }
  }
}

void SceneNode::SetSceneTime(float time) {
  Update();
  for (auto& node : mSceneTimes.GetDirectMultiNodes()) {
    PointerCast<SceneTimeNode>(node)->Set(time);
  }
}

float SceneNode::GetSceneTime() {
  return mSceneTime;
}

void SceneNode::UpdateDependencies() {
  /// Update dependencies
  for (auto& node : mTransitiveClosure) {
    if (this != node.get()) node->Update();
  }
}
