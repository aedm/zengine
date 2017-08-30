#include <include/nodes/scenenode.h>
#include <include/shaders/engineshaders.h>
REGISTER_NODECLASS(SceneNode, "Scene");

static SharedString DrawablesSlotName = make_shared<string>("Drawables");
static SharedString CameraSlotName = make_shared<string>("Camera");
static SharedString ShadowMapSizeSlotName = make_shared<string>("Shadow map size");
static SharedString SkylightDirectionSlotName = make_shared<string>("Skylight direction");
static SharedString SkylightColorSlotName = make_shared<string>("Skylight color");
static SharedString SkylightAmbientSlotName = make_shared<string>("Ambient factor");

SceneNode::SceneNode()
  : Node(NodeType::SCENE)
  , mDrawables(this, DrawablesSlotName, true)
  , mCamera(this, CameraSlotName)
  , mShadowMapSize(this, ShadowMapSizeSlotName)
  , mSkyLightDirection(this, SkylightDirectionSlotName)
  , mSkyLightColor(this, SkylightColorSlotName)
  , mSkyLightAmbient(this, SkylightAmbientSlotName)
  , mSceneTimes(NodeType::FLOAT, this, nullptr, true, false, false, false)
{}

SceneNode::~SceneNode() {
}

void SceneNode::Draw(RenderTarget* renderTarget, Globals* globals) {
  CameraNode* camera = mCamera.GetNode();
  if (camera == nullptr) return;

  /// Skylight shadow
  renderTarget->SetShadowBufferAsTarget(globals);
  OpenGL->Clear(true, true, 0xff00ff80);

  Vec3 s = mShadowMapSize.Get();
  Vec3 lightDir = mSkyLightDirection.Get().Normal();

  globals->Camera = Matrix::LookAt(-lightDir, Vec3(0, 0, 0), Vec3(0, 1, 0));
  globals->Projection = Matrix::Ortho(-s.x, -s.y, s.x, s.y, -s.z, s.z);
  globals->World.LoadIdentity();
  globals->SkylightProjection = globals->Projection;
  globals->SkylightCamera = globals->Camera;
  globals->SkylightDirection = mSkyLightDirection.Get();
  globals->SkylightColor = mSkyLightColor.Get();
  globals->SkylightAmbient = mSkyLightAmbient.Get();
  RenderDrawables(globals, PassType::SHADOW);

  /// Draw to G-Buffer
  renderTarget->SetGBufferAsTarget(globals);
  OpenGL->Clear(true, true, 0x303030);
  camera->SetupGlobals(globals);
  globals->SkylightTexture = renderTarget->mShadowTexture;
  globals->SkylightColorTexture = renderTarget->mShadowColorBuffer;
  RenderDrawables(globals, PassType::SOLID);
}

void SceneNode::RenderDrawables(Globals* globals, PassType passType) {
  for (UINT i = 0; i < mDrawables.GetMultiNodeCount(); i++) {
    Drawable* drawable = SafeCast<Drawable*>(mDrawables.GetReferencedMultiNode(i));
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
      TheMessageQueue.Enqueue(this, this, MessageType::NEEDS_REDRAW);
      break;
    case MessageType::SCENE_TIME_EDITED:
    {
      auto source = SafeCast<SceneTimeNode*>(message->mSource);
      mSceneTime = source->Get();
      for (Node* node : mSceneTimes.GetDirectMultiNodes()) {
        if (node != source) {
          SafeCast<SceneTimeNode*>(node)->Set(mSceneTime);
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
  for (Node* node : mTransitiveClosure) {
    if (IsInstanceOf<SceneTimeNode>(node)) {
      SceneTimeNode* sceneTimeNode = static_cast<SceneTimeNode*>(node);
      ASSERT(sceneTimeNode);
      mSceneTimes.Connect(sceneTimeNode);
    }
  }
}

void SceneNode::SetSceneTime(float time) {
  if (!mIsUpToDate) {
    CalculateRenderDependencies();
    mIsUpToDate = true;
  }
  for (Node* node : mSceneTimes.GetDirectMultiNodes()) {
    SafeCast<SceneTimeNode*>(node)->Set(time);
  }
}

float SceneNode::GetSceneTime() {
  return mSceneTime;
}
