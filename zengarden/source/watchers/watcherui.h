#pragma once

#include <zengine.h>
#include <QtCore/QString>
#include <QDragEnterEvent>
#include <QDropEvent>

class WatcherWidget;
class EventForwarderGLWidget;

/// Watchers show the contents of a node or edit it. They show the content as a tab on 
/// the ui, or as a property panel item, etc. Watchers belong to their WatcherWidgets, 
/// and are destroyed by them.
class WatcherUI: public Watcher {
  friend class WatcherWidget;

public:
  virtual ~WatcherUI();

  /// Get the OpenGL widget, if any
  EventForwarderGLWidget* GetGLWidget();

  /// Get displayed name
  const QString& GetDisplayedName();

  /// Handle drag events
  virtual void HandleDragEnterEvent(QDragEnterEvent* event) {}
  virtual void HandleDropEvent(QDropEvent* event) {}

  /// Set WatcherWidget that draws contents
  virtual void SetWatcherWidget(WatcherWidget* watcherWidget);

  /// Destorys the watcher and its UI elements
  virtual void Unwatch() override;

  /// Triggered when the Watcher isn't attached to a Node anymore. 
  /// Can delete WatcherWidget if necessary.
  FastDelegate<void(WatcherWidget*)> onUnwatch;

  /// Generate a name displayed for a node. If the node has no name, it shows
  /// the node type or the shader stub name.
  static QString CreateDisplayedName(Node* node);

protected:
  WatcherUI(Node* node);

  /// The watcher widget that contains this watcher
  WatcherWidget* mWatcherWidget = nullptr;

  /// The name of the node. Can be something else if the node has no name.
  QString mDisplayedName;

  virtual void OnNameChange() override;
};
