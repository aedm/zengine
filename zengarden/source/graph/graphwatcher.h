#pragma once

#include "../watchers/watcherwidget.h"
#include "../watchers/watcher.h"
#include <zengine.h>
#include <vector>

using namespace std;
class NodeWidget;
class Graph;

class GraphWatcher: public Watcher {
  friend class NodeWidget;

public:
  GraphWatcher(Graph* graph, GLWatcherWidget* Parent);

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

  /// This method will be callen when the watched graph receives an internal message
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  /// Sends an update message. Graph panel will be repainted at the next suitable moment.
  void Update();

  /// Draws the graph
  void Paint(GLWidget*);

  /// All wigdets on the graph
  Graph* GetGraph();

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

