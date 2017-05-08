#include <include/nodes/movienode.h>
#include <algorithm>

REGISTER_NODECLASS(MovieNode, "Movie");

static SharedString ClipSlotName = make_shared<string>("clips");

static const int MaxTrackCount = 8;

MovieNode::MovieNode()
  : Node(NodeType::MOVIE)
  , mClips(this, ClipSlotName, true) 
{
  mTracks.resize(MaxTrackCount);
}

MovieNode::~MovieNode() {

}

void MovieNode::Draw(RenderTarget* renderTarget) {

}

int MovieNode::GetTrackCount() {
  return int(mTracks.size());
}

const std::vector<ClipNode*>& MovieNode::GetTrack(int trackIndex) {
  return mTracks[trackIndex];
}

void MovieNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
      if (slot == &mClips) {
        SortClips();
        NotifyWatchers(&Watcher::OnRedraw);
      }
      break;
    default: break;
  }
}

void MovieNode::SortClips() {
  for (auto& track : mTracks) track.clear();
  for (Node* node : mClips.GetMultiNodes()) {
    ClipNode* clipNode = static_cast<ClipNode*>(node);
    int trackNumber = int(clipNode->mTrackNumber.Get());
    if (trackNumber < 0) trackNumber = 0;
    if (trackNumber >= mTracks.size()) trackNumber = int(mTracks.size() - 1);
    mTracks[trackNumber].push_back(clipNode);
  }

  for (auto& track : mTracks) {
    std::sort(track.begin(), track.end(), [](ClipNode* a, ClipNode* b) {
      return a->mStartTime.Get() < b->mStartTime.Get();
    });
  }
}
