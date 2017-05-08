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

void SceneNode::Draw(RenderTarget* renderTarget) {
  CameraNode* camera = mCamera.GetNode();
  if (camera == nullptr) return;
  camera->SetupGlobals(&mGlobals, renderTarget->GetSize());

  /// Skylight projection
  //Vec3 s = mShadowMapSize.Get();
  //Vec3 dir = mSkyLightDirection.Get();
  //mGlobals.SkylightMatrix = Matrix::Rotate(Quaternion::FromEuler(dir.x, dir.y, dir.z)) * Matrix::Ortho(-s.x, -s.y, s.x, s.y, -s.z, s.z);
  //Vec4 test(1, 1, 1, 1);
  //Vec4 tr = test * mGlobals.SkylightMatrix;

  /// Skylight shadow
  //TheDrawingAPI->SetFrameBuffer(renderTarget->mShadowBufferId);
  //TheDrawingAPI->SetViewport(0, 0, 512, 512);
  //TheDrawingAPI->Clear(true, true, 0xff00ff80);
  //RenderDrawables(PassType::SHADOW);

  /// Draw to G-Buffer
  renderTarget->SetGBufferAsTarget(&mGlobals);
  //mGlobals.SkylightTexture = renderTarget->mShadowTexture;
  //mGlobals.SkylightColorTexture = renderTarget->mShadowColorBuffer;
  TheDrawingAPI->Clear(true, true, 0x303030);
  RenderDrawables(PassType::SOLID);

  /// Apply post-process to scene to framebuffer
  TheEngineShaders->ApplyPostProcess(renderTarget, &mGlobals);
}

void SceneNode::RenderDrawables(PassType passType) {
  for (Node* node : mDrawables.GetMultiNodes()) {
    Drawable* drawable = static_cast<Drawable*>(node);
    drawable->Draw(&mGlobals, passType);
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
  GenerateTransitiveClosure(mTransitiveClosure);
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
