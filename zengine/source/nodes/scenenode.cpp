#include <include/nodes/scenenode.h>
#include <include/shaders/engineshaders.h>
REGISTER_NODECLASS(SceneNode, "Scene");

static SharedString DrawablesSlotName = make_shared<string>("Drawables");
static SharedString CameraSlotName = make_shared<string>("Camera");
static SharedString ShadowMapSizeSlotName = make_shared<string>("Shadow map size");
static SharedString SkylightDirectionSlotName = make_shared<string>("Skylight direction");

SceneNode::SceneNode()
  : Node(NodeType::SCENE)
  , mDrawables(this, DrawablesSlotName, true)
  , mCamera(this, CameraSlotName)
  , mShadowMapSize(this, ShadowMapSizeSlotName)
  , mSkyLightDirection(this, SkylightDirectionSlotName)
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
  Vec3 dir = mSkyLightDirection.Get();
  globals->Projection = Matrix::Ortho(-s.x, -s.y, s.x, s.y, -s.z, s.z);
  globals->Camera = Matrix::Rotate(Quaternion::FromEuler(dir.x, dir.y, dir.z));
  globals->World.LoadIdentity();
  globals->SkylightProjection = globals->Projection;
  globals->SkylightCamera = globals->Camera;
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
  for (Node* node : mDrawables.GetMultiNodes()) {
    Drawable* drawable = static_cast<Drawable*>(node);
    drawable->Draw(globals, passType);
  }
}

void SceneNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  Node::HandleMessage(message, slot, payload);
  switch (message) {
    case NodeMessage::TRANSITIVE_CLOSURE_CHANGED:
      mIsUpToDate = false;
      break;
    case NodeMessage::VALUE_CHANGED:
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      ReceiveMessage(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}

void SceneNode::CalculateRenderDependencies() {
  mTransitiveClosure.clear();
  mDependentSceneTimeNodes.clear();
  GenerateTransitiveClosure(mTransitiveClosure, true);
  for (Node* node : mTransitiveClosure) {
    if (IsInstanceOf<SceneTimeNode>(node)) {
      SceneTimeNode* sceneTimeNode = dynamic_cast<SceneTimeNode*>(node);
      ASSERT(sceneTimeNode);
      mDependentSceneTimeNodes.push_back(sceneTimeNode);
    }
  }
}

void SceneNode::SetSceneTime(float seconds) {
  if (!mIsUpToDate) {
    CalculateRenderDependencies();
    mIsUpToDate = true;
  }
  for (SceneTimeNode* sceneTimeNode : mDependentSceneTimeNodes) {
    sceneTimeNode->Set(seconds);
  }
}
