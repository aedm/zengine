#include <include/nodes/clipnode.h>

REGISTER_NODECLASS(ClipNode, "Clip");

static SharedString SceneSlotName = make_shared<string>("scene");
static SharedString StartSlotName = make_shared<string>("Start time");
static SharedString LengthSlotName = make_shared<string>("Clip Length");
static SharedString TrackNumberSlotName = make_shared<string>("Track");
static SharedString ClearColorBufferSlotName = make_shared<string>("Clear color buffer");
static SharedString ClearDepthBufferSlotName = make_shared<string>("Clear depth buffer");
static SharedString CopyToSecondaryBufferSlotName = make_shared<string>("Copy to secondary buffer");
static SharedString ApplyPostprocessBeforeSlotName = make_shared<string>("After postprocess");

ClipNode::ClipNode()
  : Node(NodeType::CLIP)
  , mSceneSlot(this, SceneSlotName)
  , mStartTime(this, StartSlotName)
  , mLength(this, LengthSlotName)
  , mTrackNumber(this, TrackNumberSlotName)
  , mClearColorBuffer(this, ClearColorBufferSlotName)
  , mClearDepthBuffer(this, ClearDepthBufferSlotName)
  , mCopyToSecondaryBuffer(this, CopyToSecondaryBufferSlotName)
  , mApplyPostprocessBefore(this, ApplyPostprocessBeforeSlotName)
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
  if (clearDepth || clearColor) {
    if (globals->DirectToScreen >= 0.5f) {
      renderTarget->SetColorBufferAsTarget(globals);
    }
    else {
      renderTarget->SetGBufferAsTarget(globals);
    }
    OpenGL->Clear(clearColor, clearDepth);
  }

  SceneNode* scene = mSceneSlot.GetNode();
  if (!scene) return;

  scene->SetSceneTime(clipTime);
  scene->Draw(renderTarget, globals);
}

void ClipNode::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::VALUE_CHANGED:
      SendMsg(MessageType::VALUE_CHANGED);
      break;
    case MessageType::SCENE_TIME_EDITED:
      SendMsg(MessageType::SCENE_TIME_EDITED);
    default: break;
  }
}
