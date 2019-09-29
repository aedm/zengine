#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class SceneNode;

class RenderForwarder: public Watcher {
public:
  RenderForwarder(const shared_ptr<SceneNode>& node);
  virtual ~RenderForwarder() = default;
  void OnRedraw() override;
  FastDelegate<void()> mOnRedraw;
};

class GeneralSceneWatcher: public WatcherUi {
public:
  GeneralSceneWatcher(const shared_ptr<Node>& node);
  virtual ~GeneralSceneWatcher();

  /// Initializes resources needed for scene watchers
  static void Init();

  /// Called when the watcher needs to be rerendered
  void OnRedraw() override;

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(EventForwarderGlWidget* widget);

  RenderTarget* mRenderTarget = nullptr;

  ///// Scene node to be drawn.
  //shared_ptr<Node> mScene;
  
  /// Default scene node, might be unused
  shared_ptr<SceneNode> mTheScene;

  /// Forwarder that catches render events for the default scene
  shared_ptr<RenderForwarder> mRenderForwarder;

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
  Vec3 mOriginalOrientation;
  float mOriginalDistance = 0.0f;

  /// Static resources initializes by Init()
  static shared_ptr<Material> mDefaultMaterial;

private:
  /// The Drawable supplied by an implementation
  shared_ptr<Drawable> mDrawable;

  Globals mGlobals;
};


