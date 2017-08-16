#include <include/nodes/movienode.h>
#include <include/shaders/engineshaders.h>
#include <algorithm>

REGISTER_NODECLASS(MovieNode, "Movie");

static SharedString ClipSlotName = make_shared<string>("clips");

static const int MaxTrackCount = 8;

MovieNode::MovieNode()
  : Node(NodeType::MOVIE)
  , mClips(this, ClipSlotName, true) {
  mTracks.resize(MaxTrackCount);
}

MovieNode::~MovieNode() {

}

void MovieNode::Draw(RenderTarget* renderTarget, float time) {
  bool clipFound = false;
  for (vector<ClipNode*>& track : mTracks) {
    int clipIndex = 0;
    int clipCount = int(track.size());
    for (; clipIndex < clipCount; clipIndex++) {
      ClipNode* clip = track[clipIndex];
      float startTime = clip->mStartTime.Get();
      float endTime = startTime + clip->mLength.Get();
      if (startTime <= time && endTime > time) {
        /// Render clip
        clip->Draw(renderTarget, &mGlobals, time - startTime);
        clipFound = true;
        break;
      }
      if (startTime > time) break;
    }
  }
  
  if (clipFound) {
    /// Apply post-process to scene to framebuffer
    TheEngineShaders->ApplyPostProcess(renderTarget, &mGlobals);
  }
  else {
    renderTarget->SetColorBufferAsTarget(&mGlobals);
    OpenGL->Clear(true, true);
  }
}

int MovieNode::GetTrackCount() {
  return int(mTracks.size());
}

const std::vector<ClipNode*>& MovieNode::GetTrack(int trackIndex) {
  return mTracks[trackIndex];
}

float MovieNode::CalculateMovieLength() {
  float length = 0.0f;
  for (auto& track : mTracks) {
    for (ClipNode* clipNode : track) {
      float clipEnd = clipNode->mStartTime.Get() + clipNode->mLength.Get();
      if (clipEnd > length) length = clipEnd;
    }
  }
  return length;
}

void MovieNode::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
    case MessageType::VALUE_CHANGED:
      if (message->mSlot == &mClips) {
        SortClips();
        NotifyWatchers(&Watcher::OnRedraw);
      }
      break;
    case MessageType::SCENE_TIME_EDITED:
    {
      ClipNode* clipNode = SafeCast<ClipNode*>(message->mSource);
      SceneNode* scene = clipNode->mSceneSlot.GetNode();
      if (!scene) return;
      float time = clipNode->mStartTime.Get() + scene->GetSceneTime();
      NotifyWatchers(&Watcher::OnTimeEdited, time);
      break;
    }
    default: break;
  }
}

void MovieNode::SortClips() {
  for (auto& track : mTracks) track.clear();
  for (UINT i = 0; i < mClips.GetMultiNodeCount(); i++) {
    ClipNode* clipNode = static_cast<ClipNode*>(mClips.GetReferencedMultiNode(i));
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
