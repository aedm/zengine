#pragma once

#include "../watchers/watcherwidget.h"
#include "../watchers/watcher.h"
#include <zengine.h>
#include <vector>

using namespace std;
class NodeWidget;
class GraphNode;

class GraphWatcher: public Watcher {
  friend class CreateNodeCommand;
  friend class NodeWidget;

public:
  GraphWatcher(GraphNode* graph, GLWatcherWidget* Parent);

  NodeWidget* GetNodeWidget(Node* node);

private:
  /// Qt widget event handlers
  void HandleMousePress(GLWidget*, QMouseEvent* event);
  void HandleMouseRelease(GLWidget*, QMouseEvent* event);
  void HandleMouseMove(GLWidget*, QMouseEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(GLWidget*, QKeyEvent* event);

  void HandleGraphNeedsRepaint();

  /// Draws the graph
  void Paint(GLWidget*);

  /// Will be called when a widget wants to repaint
  void HandleWidgetRepaint();

  /// Command-accessible functions
  NodeWidget* AddNode(Node* node);

  /// All wigdets on the graph
  GraphNode* GetGraph();

  /// Mapping from node to widget
  map<Node*, NodeWidget*> mWidgetMap;

  /// Operators currectly selected
  set<NodeWidget*> mSelectedNodes;

  /// State machine
  enum class State {
    DEFAULT,
    MOVE_NODES,
    SELECT_RECTANGLE,
    CONNECT_TO_NODE,
    CONNECT_TO_SLOT,
  };
  State mCurrentState;

  /// True if mouse movement was made during STATE_MOVE_NODES
  bool mAreNodesMoved;

  /// True if connection is estimated to be valid
  bool mIsConnectionValid;

  /// Mouse position when the clicking occured
  Vec2 mOriginalMousePos;

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

