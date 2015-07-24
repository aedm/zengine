#pragma once

#include "../util/textTexture.h"
#include "graphwatcher.h"
#include "../watchers/watcher.h"
#include <zengine.h>

/// NodeWidget is a Watcher for a Node on a Graph. Every Node has a NodeWidget associated
/// to it.

class NodeWidget: public Watcher {
  friend class MoveNodeCommand;
  friend class GraphWatcher;

public:
  NodeWidget(Node* node, GraphWatcher* graphWatcher);
  ~NodeWidget();

  void Paint();
  Vec2 GetOutputPosition();
  Vec2 GetInputPosition(int slotIndex);

  struct WidgetSlot {
    TextTexture mTexture;
    Vec2 mPosition;
    Vec2 mSize;
    Vec2 mSpotPos;
    Slot* mSlot;
  };

  vector<WidgetSlot*>	mWidgetSlots;

private:
  virtual void HandleSniffedMessage(NodeMessage Message, Slot* S,
                                    void* Payload) override;

  /// Handles node title change
  void HandleTitleChange();

  /// Layout
  void CalculateLayout();

  //Vec2 mPosition;
  //Vec2 mSize;
  Vec2 mOutputPosition;

  /// Viewer states
  bool mIsSelected;
  Vec2 mOriginalPosition;
  Vec2 mOriginalSize;

  /// Height of the titlebar
  float mTitleHeight;

  void CreateWidgetSlots();

  TextTexture* mTitleTexture;

  GraphWatcher* mGraphWatcher;
};

