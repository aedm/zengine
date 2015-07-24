#pragma once

#include <zengine.h>
#include <QtCore/QString>

class WatcherWidget;
class GLWidget;

/// Watchers show the contents of a node or edit it. They show the content as a tab on 
/// the ui, or as a property panel item, etc. Watchers belong to their WatcherWidgets, 
/// and are destroyed by them.
class Watcher {
public:
  virtual ~Watcher();

  /// Returns the node being watched
  Node*	GetNode();

  /// Change the displayed node to another
  void ChangeNode(Node* node);

  /// Get the OpenGL widget, if any
  GLWidget* GetGLWidget();

  /// Get displayed name
  const QString& GetDisplayedName();

protected:
  Watcher(Node* node, WatcherWidget* watcherWidget, NodeType type = NodeType::UI);

  /// This method will be called when the watched node was changed. The node parameter 
  /// is null if the underlying node is being deleted.
  virtual void HandleChangedNode(Node* node);

  /// This method will be callen when the watched node receives an internal message
  virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, void* payload);

  /// The node beign watched
  //Slot mWatchedNode;
  Node* mNode;
  
  /// The watcher widget that contains this watcher
  WatcherWidget* mWatcherWidget;

  /// The name of the node. Can be something else if the node has no name.
  QString mDisplayedName;

private:
  /// Helper function for forwaring internal messages of the watched node
  void SniffMessage(Slot* slot, NodeMessage message, void* payload);

  /// Generate the name displayed for the node. If the node has no name, it shows
  /// the node type or the shader stub name.
  void MakeDisplayedName();
};
