#pragma once

#include <zengine.h>

class WatcherWidget;

/// A node that displays another node. The display can be a tab on the ui, or a property
/// panel item, etc. Watchers belong to their WatcherWidgets, and are destroyed by
/// them.
class Watcher: public Node {
public:
  Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type = NodeType::UI);
  virtual ~Watcher();

  Node*	GetNode();

protected:
  virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, const void* payload);
  Slot mWatchedNode;
  WatcherWidget* mWatcherWidget;

private:
  void SniffMessage(Slot* slot, NodeMessage message, const void* payload);
};
