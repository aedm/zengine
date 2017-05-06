#include <include/nodes/clipnode.h>

REGISTER_NODECLASS(ClipNode, "Clip");

static SharedString SceneSlotName = make_shared<string>("scene");
static SharedString StartSlotName = make_shared<string>("Start time");
static SharedString LengthSlotName = make_shared<string>("Clip Length");
static SharedString TrackNumberSlotName = make_shared<string>("Track");

ClipNode::ClipNode()
  : Node(NodeType::CLIP) 
  , mSceneSlot(this, SceneSlotName)
  , mStartTime(this, StartSlotName)
  , mLength(this, LengthSlotName)
  , mTrackNumber(this, TrackNumberSlotName)
{

}

ClipNode::~ClipNode() {

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
