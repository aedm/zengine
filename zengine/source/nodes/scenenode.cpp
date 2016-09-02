#include <include/nodes/scenenode.h>
#include <include/shaders/engineshaders.h>

REGISTER_NODECLASS(SceneNode, "Scene");

static SharedString DrawablesSlotName = make_shared<string>("Drawables");
static SharedString CameraSlotName = make_shared<string>("Camera");

SceneNode::SceneNode()
  : Node(NodeType::SCENE)
  , mDrawables(this, DrawablesSlotName, true)
  , mCamera(this, CameraSlotName)
{}

SceneNode::~SceneNode() {
}

void SceneNode::Draw(RenderTarget* renderTarget) {
  CameraNode* camera = mCamera.GetNode();
  if (camera == nullptr) return;

  camera->SetupGlobals(&mGlobals, renderTarget->GetSize());

  /// Draw to G-Buffer
  renderTarget->SetGBufferAsTarget(&mGlobals);
  TheDrawingAPI->Clear(true, true, 0x303030);
  RenderDrawables();

  /// Apply post-process to scene to framebuffer
  TheEngineShaders->ApplyPostProcess(renderTarget, &mGlobals);
}

void SceneNode::RenderDrawables() {
  for (Node* node : mDrawables.GetMultiNodes()) {
    Drawable* drawable = static_cast<Drawable*>(node);
    drawable->Draw(&mGlobals);
  }
}

void SceneNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
    case NodeMessage::NEEDS_REDRAW:
      SendMsg(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}
