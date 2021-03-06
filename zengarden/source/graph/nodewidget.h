#pragma once

#include "../watchers/watcherui.h"
#include <zengine.h>
#include <memory>
#include <functional>
#include <QPainter>

/// NodeWidget is a Watcher for a Node on a Graph. Every Node has a NodeWidget associated
/// to it.

class NodeWidget: public WatcherUi {
  friend class MoveNodeCommand;

public:
  NodeWidget(const std::shared_ptr<Node>& node, std::function<void()> onNeedsRedraw);
  virtual ~NodeWidget();

  enum class ColorState {
    DEFAULT,
    HOVERED,
    CONNECTION_FROM,
    VALID_CONNECTION,
    INVALID_CONNECTION,
  };

  /// Set the Node widget's frame color
  void SetFrameColor(ColorState frameColorState);

  /// Highlights a slot (eg. when connecting them)
  void SetSlotColor(int slotIndex, ColorState slotColorState);

  void SetSelected(bool isSelected);
  bool IsSelected() const;

  bool NeedsRepaint() const;
  void Paint();

  vec2 GetOutputPosition() const;
  vec2 GetInputPosition(int slotIndex);

  /// Slot paint data
  struct WidgetSlot {
    vec2 mPosition = vec2();
    vec2 mSize = vec2();
    vec2 mSpotPos = vec2();
    Slot* mSlot = nullptr;
  };
  const std::vector<WidgetSlot*>& GetWidgetSlots() const;

private:
  void OnSlotStructureChanged() override;
  void OnNameChange() override;
  void OnGraphPositionChanged() override;
  void OnSlotConnectionChanged(Slot* slot) override;

  void HandleNameChange();
    
  /* -------- drawing parameters ---------- */

  /// Texture rendeing parameters
  struct PaintState {
    ColorState mFrameColorState = ColorState::DEFAULT;
    ColorState mSlotColorState = ColorState::DEFAULT;
    int mColoredSlotIndex = -1;
    bool mIsSelected = false;

    /// Index of the currently highlighted slot
    int mHighlightedSlotIndex = -1;
  };
  
  /// Currently rendered image's parameters
  PaintState mCurrentPaintState;

  /// The widget's next paint state
  PaintState mNextPaintState;


  /* -------- layout ---------- */

  void CalculateLayout();
  void CreateWidgetSlots();

  /// Height of the titlebar
  float mTitleHeight = 0.0f;
  vec2 mOutputPosition;
  QString mNodeTitle;
  std::vector<WidgetSlot*>	mWidgetSlots;

  /// This will be called if the widget initiates drawing
  std::function<void()> mOnNeedsRedraw;

  
  /* -------- paint ---------- */
  
  /// Image representation of the widget
  QImage mImage;

  /// Painter used to draw the widget texture
  QPainter mPainter;

  std::shared_ptr<Texture> mTexture = nullptr;
  bool mForceUpdate = true;
  void DiscardTexture();
  void PaintToImage();
  void UpdateTexture();
};

