#include <include/nodes/scenenode.h>

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

void SceneNode::Draw(const Vec2& canvasSize) {
  Camera* camera = mCamera.GetNode();
  ASSERT(camera != nullptr);
  camera->SetupGlobals(&mGlobals, canvasSize);

  for (Node* node : mDrawables.GetMultiNodes()) {
    Drawable* drawable = static_cast<Drawable*>(node);
    drawable->Draw(&mGlobals);
  }
}
