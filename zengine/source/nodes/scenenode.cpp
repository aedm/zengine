#include <include/nodes/scenenode.h>

REGISTER_NODECLASS(SceneNode, "Scene");

static SharedString DrawablesSlotName = make_shared<string>("Drawables");

SceneNode::SceneNode()
  : Node(NodeType::SCENE)
  , mDrawables(this, DrawablesSlotName, true)
{
}

SceneNode::~SceneNode() {
}

void SceneNode::Draw(const Vec2& canvasSize) {
  mGlobals.RenderTargetSize = canvasSize;
  mGlobals.RenderTargetSizeRecip = Vec2(1.0f / canvasSize.x, 1.0f / canvasSize.y);

  for (Node* node : mDrawables.GetMultiNodes()) {
    Drawable* drawable = static_cast<Drawable*>(node);
    drawable->Draw(&mGlobals);
  }
}
