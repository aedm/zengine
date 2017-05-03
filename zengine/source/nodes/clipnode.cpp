#include <include/nodes/clipnode.h>

REGISTER_NODECLASS(ClipNode, "Clip");

static SharedString SceneSlotName = make_shared<string>("scene");
static SharedString StartSlotName = make_shared<string>("Start beat");
static SharedString EndSlotName = make_shared<string>("End beat");
static SharedString TrackNumberSlotName = make_shared<string>("Track");

ClipNode::ClipNode()
  : Node(NodeType::CLIP) 
  , mSceneSlot(this, SceneSlotName)
  , mStartTime(this, StartSlotName)
  , mEndTime(this, EndSlotName)
  , mTrackNumber(this, TrackNumberSlotName)
{

}

ClipNode::~ClipNode() {

}
