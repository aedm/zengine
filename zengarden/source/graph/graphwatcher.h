#pragma once

#include "../watchers/watcherwidget.h"
#include "../watchers/watcherui.h"
#include <zengine.h>
#include <vector>

using namespace std;
class NodeWidget;
class Graph;

class GraphWatcher: public WatcherUI {
  friend class NodeWidget;

public:
  GraphWatcher(Graph* graph, GLWatcherWidget* Parent);
  virtual ~GraphWatcher();

  NodeWidget* GetNodeWidget(Node* node);

private:
  /// Qt widget event handlers
  void HandleMousePress(GLWidget*, QMouseEvent* event);
  void HandleMouseRelease(GLWidget*, QMouseEvent* event);
  void HandleMouseMove(GLWidget*, QMouseEvent* event);
  void HandleMouseWheel(GLWidget*, QWheelEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(GLWidget*, QKeyEvent* event);

  /// Watcher callbacks
  virtual void OnMultiSlotConnectionAdded(Slot* slot, Node* addedNode) override;

  /// Remove node widget when it's deleted
  void RemoveNodeWidget(Node* node);

  /// Sends an update message. Graph panel will be repainted at the next suitable moment.
  void Update();

  /// Draws the graph
  void Paint(GLWidget*);

  /// All wigdets on the graph
  Graph* GetGraph();

  /// Handle drag events
  virtual void HandleDragEnterEvent(QDragEnterEvent* event) override;
  virtual void HandleDropEvent(QDropEvent* event) override;

  /// Mapping from node to widget
  map<Node*, NodeWidget*> mWidgetMap;

  /// Operators currectly selected
  set<NodeWidget*> mSelectedNodeWidgets;

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
  Vec2 mCenter = Vec2(0, 0);
  float mZoomFactor = 1.0f;
  int mZoomExponent = 0;

  /// Convertes pixel coordinates (eg. mouse) to world coordinates
  Vec2 MouseToWorld(QMouseEvent* event);
  Vec2 CanvasToWorld(const Vec2& canvasCoord);
  void GetCanvasDimensions(Vec2& oCanvasSize, Vec2& oTopLeft);

  /// True if mouse movement was made during STATE_MOVE_NODES
  bool mAreNodesMoved;

  /// True if connection is estimated to be valid
  bool mIsConnectionValid;

  /// Mouse position when the clicking occured
  Vec2 mOriginalMousePos;

  /// Original position of center at the time of pressing the left mouse button
  Vec2 mOriginalCenter;
  
  /// Current mouse position
  Vec2 mCurrentMousePos;

  /// Clicked widget for some operations
  NodeWidget* mClickedWidget;
  int mClickedSlotIndex;

  /// Hovered widget and slot
  NodeWidget* mHoveredWidget;
  int mHoveredSlotIndex;

  void DeselectAll();
  void StorePositionOfSelectedNodes();

  /// Finds which widget and slot is hovered by the mouse pointer.
  /// Return true if either the hovered node or the hoeverd slot changed.
  bool UpdateHoveredWidget(Vec2 mousePos);
};

