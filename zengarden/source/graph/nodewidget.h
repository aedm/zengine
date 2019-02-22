#pragma once

#include "../util/textTexture.h"
#include "graphwatcher.h"
#include "../watchers/watcherui.h"
#include <zengine.h>
#include <memory>
#include <QImage>
#include <QPainter>

/// NodeWidget is a Watcher for a Node on a Graph. Every Node has a NodeWidget associated
/// to it.

class NodeWidget: public WatcherUI {
  friend class MoveNodeCommand;
  //friend class GraphWatcher;

public:
  NodeWidget(const shared_ptr<Node>& node, GraphWatcher* graphWatcher);
  virtual ~NodeWidget();

  void Paint();
  Vec2 GetOutputPosition();
  Vec2 GetInputPosition(int slotIndex);

  void SetSelected(bool isSelected);
  bool IsSelected();

  struct WidgetSlot {
    Vec2 mPosition;
    Vec2 mSize;
    Vec2 mSpotPos;
    Slot* mSlot;
  };

  vector<WidgetSlot*>	mWidgetSlots;

  /// Original positions in move/resize operations
  /// TODO: remove these.
  Vec2 mOriginalPosition;
  Vec2 mOriginalSize;

private:
  virtual void OnSlotStructureChanged() override;
  virtual void OnNameChange() override;
  virtual void OnGraphPositionChanged() override;
  virtual void OnSlotConnectionChanged(Slot* slot) override;

  /// Layout
  void CalculateLayout();

  void UpdateGraph();

  void UpdateTexture();

  //Vec2 mPosition;
  //Vec2 mSize;
  Vec2 mOutputPosition;

  /// Viewer states
  bool mIsSelected;

  QString mNodeTitle;

  /// Height of the titlebar
  float mTitleHeight;

  void CreateWidgetSlots();

  //TextTexture* mTitleTexture;

  GraphWatcher* mGraphWatcher;

  /// Image representation of the widget
  QImage mImage;

  /// Painter used to draw the widget texture
  QPainter mPainter;

  Texture* mTexture = nullptr;
  bool mUptodate = false;
  void DiscardTexture();

  void PaintToImage();
};

