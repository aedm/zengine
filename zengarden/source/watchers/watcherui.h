#pragma once

#include <zengine.h>
#include <QtCore/QString>
#include <QDragEnterEvent>
#include <QDropEvent>

class WatcherWidget;
class GLWidget;

/// Watchers show the contents of a node or edit it. They show the content as a tab on 
/// the ui, or as a property panel item, etc. Watchers belong to their WatcherWidgets, 
/// and are destroyed by them.
class WatcherUI: public Watcher {
public:
  virtual ~WatcherUI();

  /// Returns the node being watched
  Node*	GetNode();

  /// Get the OpenGL widget, if any
  GLWidget* GetGLWidget();

  /// Get displayed name
  const QString& GetDisplayedName();

  /// Handle drag events
  virtual void HandleDragEnterEvent(QDragEnterEvent* event) {}
  virtual void HandleDropEvent(QDropEvent* event) {}

  /// The watcher widget that contains this watcher
  WatcherWidget* mWatcherWidget;

protected:
  WatcherUI(Node* node, WatcherWidget* watcherWidget, NodeType type = NodeType::UI);

  /// The name of the node. Can be something else if the node has no name.
  QString mDisplayedName;

  virtual void OnNameChange() override;

private:
  /// Generate the name displayed for the node. If the node has no name, it shows
  /// the node type or the shader stub name.
  void MakeDisplayedName();
};
