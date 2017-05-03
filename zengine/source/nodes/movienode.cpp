#include <include/nodes/movienode.h>

REGISTER_NODECLASS(MovieNode, "Movie");

static SharedString ClipSlotName = make_shared<string>("clips");

MovieNode::MovieNode()
  : Node(NodeType::MOVIE)
  , mClips(this, ClipSlotName, true)
{

}

MovieNode::~MovieNode() {

}
