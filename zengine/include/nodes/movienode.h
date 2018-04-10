#pragma once

#include "../dom/node.h"
#include "clipnode.h"

class MovieNode: public Node {
public:
  MovieNode();
  virtual ~MovieNode();

  ClipSlot mClips;

  void Draw(RenderTarget* renderTarget, float time);

  int GetTrackCount();
  const std::vector<shared_ptr<ClipNode>>& GetTrack(int trackIndex);

  /// Returns the end of the last clip
  float CalculateMovieLength();

protected:
  virtual void HandleMessage(Message* message) override;

private:
  // Tracks ordered by starting time of clips
  vector<vector<shared_ptr<ClipNode>>> mTracks;

  void SortClips();

  Globals mGlobals;
};

typedef TypedSlot<MovieNode> MovieSlot;
