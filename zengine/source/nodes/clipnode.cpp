#include <include/nodes/clipnode.h>

REGISTER_NODECLASS(ClipNode, "Clip");

static SharedString SceneSlotName = make_shared<string>("scene");
static SharedString StartSlotName = make_shared<string>("Start time");
static SharedString LengthSlotName = make_shared<string>("Clip Length");
static SharedString TrackNumberSlotName = make_shared<string>("Track");
static SharedString ClearColorBufferSlotName = make_shared<string>("Clear color buffer");
static SharedString ClearDepthBufferSlotName = make_shared<string>("Clear depth buffer");
static SharedString CopyToSecondaryBufferSlotName = make_shared<string>("Copy to secondary buffer");

ClipNode::ClipNode()
  : Node(NodeType::CLIP)
  , mSceneSlot(this, SceneSlotName)
  , mStartTime(this, StartSlotName)
  , mLength(this, LengthSlotName)
  , mTrackNumber(this, TrackNumberSlotName)
  , mClearColorBuffer(this, ClearColorBufferSlotName)
  , mClearDepthBuffer(this, ClearDepthBufferSlotName)
  , mCopyToSecondaryBuffer(this, CopyToSecondaryBufferSlotName)
{}

ClipNode::~ClipNode() {

}

void ClipNode::Draw(RenderTarget* renderTarget, Globals* globals, float clipTime) {
  if (mCopyToSecondaryBuffer.Get() >= 0.5f) {
    int width = int(globals->RenderTargetSize.x);
    int height = int(globals->RenderTargetSize.y);
    OpenGL->BlitFrameBuffer(renderTarget->mGBufferId, renderTarget->mSecondaryFramebuffer, 
      0, 0, width, height, 0, 0, width, height);
    OpenGL->SetFrameBuffer(renderTarget->mSecondaryFramebuffer);
    OpenGL->SetFrameBuffer(renderTarget->mGBufferId);
  }
  
  bool clearColor = mClearColorBuffer.Get() >= 0.5f;
  bool clearDepth = mClearDepthBuffer.Get() >= 0.5f;
  OpenGL->Clear(clearColor, clearDepth);

  SceneNode* scene = mSceneSlot.GetNode();
  if (!scene) return;

  scene->SetSceneTime(clipTime);
  scene->Draw(globals);
}

void ClipNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::VALUE_CHANGED:
      SendMsg(NodeMessage::VALUE_CHANGED);
      break;
    default:
      Node::HandleMessage(message, slot, payload);
      break;
  }
}
