#pragma once

#include "../util/textTexture.h"
#include "graphwatcher.h"
#include "../watchers/watcherui.h"
#include <zengine.h>
#include <memory>
#include <QImage>

/// NodeWidget is a Watcher for a Node on a Graph. Every Node has a NodeWidget associated
/// to it.

class NodeWidget: public WatcherUI {
  friend class MoveNodeCommand;
  friend class GraphWatcher;

public:
  NodeWidget(const shared_ptr<Node>& node, GraphWatcher* graphWatcher);
  virtual ~NodeWidget();

  void Paint();
  Vec2 GetOutputPosition();
  Vec2 GetInputPosition(int slotIndex);

  struct WidgetSlot {
    //TextTexture mTexture;
    Vec2 mPosition;
    Vec2 mSize;
    Vec2 mSpotPos;
    Slot* mSlot;
  };

  vector<WidgetSlot*>	mWidgetSlots;

private:
  virtual void OnSlotStructureChanged() override;
  virtual void OnNameChange() override;
  virtual void OnGraphPositionChanged() override;
  virtual void OnSlotConnectionChanged(Slot* slot) override;

  /// Layout
  void CalculateLayout();

  void UpdateGraph();

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

  //TextTexture* mTitleTexture;

  GraphWatcher* mGraphWatcher;

  /// Image representation of the widget
  unique_ptr<QImage> mImage;
  Texture* mTexture = nullptr;
  bool mUptodate = false;
  void DiscardTexture();

  void PaintToImage();
};

