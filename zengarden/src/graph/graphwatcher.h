#pragma once

#include "../watchers/watcherwidget.h"
#include "../watchers/watcherui.h"
#include <zengine.h>

class NodeWidget;
class Graph;
class Node;

class GraphWatcher: public WatcherUi {
  friend class NodeWidget;
  friend class Node; 

public:
  GraphWatcher(const std::shared_ptr<Node>& graph);
  virtual ~GraphWatcher();

  std::shared_ptr<NodeWidget> GetNodeWidget(const std::shared_ptr<Node>& node);

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

private:
  /// Qt widget event handlers
  void HandleMousePress(EventForwarderGlWidget*, QMouseEvent* event);
  void HandleMouseRelease(EventForwarderGlWidget*, QMouseEvent* event);
  void HandleMouseMove(EventForwarderGlWidget*, QMouseEvent* event);
  void HandleMouseWheel(EventForwarderGlWidget*, QWheelEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(EventForwarderGlWidget*, QKeyEvent* event);

  /// Watcher callbacks
  void OnSlotConnectionChanged(Slot* slot) override;

  /// Sends an update message. Graph panel will be repainted at the next suitable moment.
  void Update() const;

  /// Draws the graph
  void Paint(EventForwarderGlWidget*);

  /// All widgets on the graph
  std::shared_ptr<Graph> GetGraph() const;

  /// Handle drag events
  void HandleDragEnterEvent(QDragEnterEvent* event) override;
  void HandleDropEvent(QDropEvent* event) override;

  /// Mapping from node to widget
  std::map<std::shared_ptr<Node>, std::shared_ptr<NodeWidget>> mWidgetMap;

  /// Operators currently selected
  std::set<std::shared_ptr<NodeWidget>> mSelectedNodeWidgets;

  /// State machine
  enum class State {
    DEFAULT,
    MOVE_NODES,
    SELECT_RECTANGLE,
    CONNECT_TO_NODE,
    CONNECT_TO_SLOT,
    PAN_CANVAS,
  };
  State mCurrentState;

  /// View parameters
  vec2 mCenter = vec2(0, 0);
  float mZoomFactor = 1.0f;
  int mZoomExponent = 0;

  /// Converts pixel coordinates (eg. mouse) to world coordinates
  vec2 MouseToWorld(QMouseEvent* event) const;
  vec2 CanvasToWorld(const vec2& canvasCoord) const;
  void GetCanvasDimensions(vec2& oCanvasSize, vec2& oTopLeft) const;
  
  /// True if mouse movement was made during STATE_MOVE_NODES
  bool mAreNodesMoved{};

  /// True if connection is estimated to be valid
  bool mIsConnectionValid{};

  /// Mouse position when the clicking occured
  vec2 mOriginalMousePos;

  /// Original position of center at the time of pressing the left mouse button
  vec2 mOriginalCenter;
  
  /// Current mouse position
  vec2 mCurrentMousePos;

  /// Clicked widget for some operations
  std::shared_ptr<NodeWidget> mClickedWidget;
  int mClickedSlotIndex{};

  /// Hovered widget and slot
  std::shared_ptr<NodeWidget> mHoveredWidget;
  int mHoveredSlotIndex;

  /// Operators currently selected
  std::map<std::shared_ptr<NodeWidget>, vec2> mOriginalWidgetPositions;


  void DeselectAll();
  void SelectSingleWidget(const std::shared_ptr<NodeWidget>& nodeWidget);
  void StorePositionOfSelectedNodes();

  void FindHoveredWidget(vec2 mousePos, std::shared_ptr<NodeWidget>* oHoveredWidget,
    int* oSlotIndex);

  /// NodeWidgets can request a redraw
  void HandleNodeWidgetRedraw();
  bool mRedrawRequested = false;

  /// Check if any of the widgets needs repaint
  bool NeedsWidgetRepaint();
};

