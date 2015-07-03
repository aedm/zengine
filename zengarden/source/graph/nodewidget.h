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
  NodeWidget(Node* Nd);
  ~NodeWidget();

  void Paint(GraphWatcher* panel);
  Vec2 GetOutputPosition();
  Vec2 GetInputPosition(int slotIndex);

  Event<> OnRepaint;

private:
  virtual void HandleSniffedMessage(Slot* S, NodeMessage Message,
                                    const void* Payload) override;

  /// Handles node title change
  void HandleTitleChange();

  /// Command-accessible
  //void SetPosition(Vec2 position);

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

  struct WidgetSlot {
    TextTexture mTexture;
    Vec2 mPosition;
    Vec2 mSize;
    Vec2 mSpotPos;
    Slot* mSlot;
  };

  vector<WidgetSlot*>	mWidgetSlots;
  void CreateWidgetSlots();

  TextTexture* mTitleTexture;
};

