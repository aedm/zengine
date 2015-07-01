#pragma once

#include <zengine.h>

class WatcherWidget;
class GLWidget;

/// A node that displays another node. The display can be a tab on the ui, or a property
/// panel item, etc. Watchers belong to their WatcherWidgets, and are destroyed by
/// them.
class Watcher: public Node {
public:
  Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type = NodeType::UI);
  virtual ~Watcher();

  Node*	GetNode();

  /// Change the displayed node to another
  void ChangeNode(Node* node);

  /// Get the OpenGL widget, if any
  GLWidget* GetGLWidget();

protected:
  /// This method will be called when the watched node was changed
  virtual void HandleChangedNode(Node* node);

  /// This method will be callen when the watched node receives an internal message
  virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, const void* payload);

  /// The node beign watched
  Slot mWatchedNode;
  
  /// The watcher widget that contains this watcher
  WatcherWidget* mWatcherWidget;

private:
  /// Helper function for forwaring internal messages of the watched node
  void SniffMessage(Slot* slot, NodeMessage message, const void* payload);
};
