#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class SceneNode;

class RenderForwarder: public Watcher {
public:
  explicit RenderForwarder(const std::shared_ptr<SceneNode>& node);
  void OnRedraw() override;
  FastDelegate<void()> mOnRedraw;
};

class GeneralSceneWatcher: public WatcherUi {
public:
  GeneralSceneWatcher(const std::shared_ptr<Node>& node);
  ~GeneralSceneWatcher() override;

  /// Initializes resources needed for scene watchers
  static void Init();

  /// Called when the watcher needs to be rerendered
  void OnRedraw() override;

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(EventForwarderGlWidget* widget);

  RenderTarget* mRenderTarget = nullptr;

  /// Default scene node, might be unused
  std::shared_ptr<SceneNode> mTheScene;

  /// Forwarder that catches render events for the default scene
  std::shared_ptr<RenderForwarder> mRenderForwarder;

  /// Qt widget event handlers
  void HandleMousePress(EventForwarderGlWidget*, QMouseEvent* event);
  void HandleMouseRelease(EventForwarderGlWidget*, QMouseEvent* event);
  void HandleMouseMove(EventForwarderGlWidget*, QMouseEvent* event) const;
  void HandleMouseWheel(EventForwarderGlWidget*, QWheelEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(EventForwarderGlWidget*, QKeyEvent* event) const;

  /// Mouse position at pressing
  QPoint mOriginalPosition;
  vec3 mOriginalOrientation {};
  float mOriginalDistance = 0.0f;

  /// Static resources initializes by Init()
  static std::shared_ptr<Material> mDefaultMaterial;

private:
  /// The Drawable supplied by an implementation
  std::shared_ptr<Drawable> mDrawable;

  Globals mGlobals;
};


