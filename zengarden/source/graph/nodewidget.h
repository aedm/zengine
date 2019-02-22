#pragma once

#include "../util/textTexture.h"
#include "graphwatcher.h"
#include "../watchers/watcherui.h"
#include <zengine.h>
#include <memory>
#include <functional>
#include <QImage>
#include <QPainter>

/// NodeWidget is a Watcher for a Node on a Graph. Every Node has a NodeWidget associated
/// to it.

class NodeWidget: public WatcherUI {
  friend class MoveNodeCommand;

public:
  NodeWidget(const shared_ptr<Node>& node, const std::function<void()>& onNeedsRedraw);
  virtual ~NodeWidget();

  enum class FrameColor {
    DEFAULT,
    SELECTED,
    HOVERED,
    CONNECTION_FROM,
    VALID_CONNECTION,
    INVALID_CONNECTION,
  };

  /// Set the Node widget's frame color
  void SetFrameColor(FrameColor frameColor);

  enum class SlotColor {
    DEFAULT,
    HOVERED,
    CONNECTION_FROM,
    VALID_CONNECTION,
    INVALID_CONNECTION,
  };
  /// Highlights a slot (eg. when connecting them)
  void SetSlotColor(int slotIndex, SlotColor slotColor);

  void Paint();
  Vec2 GetOutputPosition();
  Vec2 GetInputPosition(int slotIndex);

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

  void UpdateTexture();

  //Vec2 mPosition;
  //Vec2 mSize;
  Vec2 mOutputPosition;

  /// Viewer states
  FrameColor mFrameColor = FrameColor::DEFAULT;
  SlotColor mSlotColor = SlotColor::DEFAULT;
  int mColoredSlotIndex = -1;

  /// Index of the currently highlighted slot
  int mHighlightedSlotIndex = -1;

  QString mNodeTitle;

  /// Height of the titlebar
  float mTitleHeight;

  void CreateWidgetSlots();

  //TextTexture* mTitleTexture;

  //weak_ptr<GraphWatcher> mGraphWatcher;
  std::function<void()> mOnNeedsRedraw;

  /// Image representation of the widget
  QImage mImage;

  /// Painter used to draw the widget texture
  QPainter mPainter;

  Texture* mTexture = nullptr;
  bool mUptodate = false;
  void DiscardTexture();

  void PaintToImage();
};

