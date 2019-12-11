#pragma once

#include <zengine.h>
#include <QtGui/QDropEvent>

class WatcherWidget;
class EventForwarderGlWidget;

/// Watchers show the contents of a node or edit it. They show the content as a tab on 
/// the ui, or as a property panel item, etc. Watchers belong to their WatcherWidgets, 
/// and are destroyed by them.
class WatcherUi: public Watcher {
  friend class WatcherWidget;

public:
  virtual ~WatcherUi();

  /// Get the OpenGL widget, if any
  EventForwarderGlWidget* GetGlWidget() const;

  /// Get displayed name
  const QString& GetDisplayedName() const;

  /// Handle drag events
  virtual void HandleDragEnterEvent(QDragEnterEvent* event) {}
  virtual void HandleDropEvent(QDropEvent* event) {}

  /// Set WatcherWidget that draws contents
  virtual void SetWatcherWidget(WatcherWidget* watcherWidget);

  /// Destroys the watcher and its UI elements
  void OnRemovedFromNode() override;

  FastDelegate<void(WatcherWidget*)> mDeleteWatcherWidgetCallback;

  /// Generate a name displayed for a node. If the node has no name, it shows
  /// the node type or the shader stub name.
  static QString CreateDisplayedName(const std::shared_ptr<Node>& node);

  /// The watcher widget that contains this watcher
  WatcherWidget* mWatcherWidget = nullptr;

protected:
  WatcherUi(const std::shared_ptr<Node>& node);

  /// The name of the node. Can be something else if the node has no name.
  QString mDisplayedName;

  void OnNameChange() override;
};
