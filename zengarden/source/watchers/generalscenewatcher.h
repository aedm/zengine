#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class SceneNode;

class RenderForwarder: public Watcher {
public:
  RenderForwarder(const shared_ptr<SceneNode>& node);
  virtual ~RenderForwarder() {}
  virtual void OnRedraw();
  FastDelegate<void()> mOnRedraw;
};

class GeneralSceneWatcher: public WatcherUI {
public:
  GeneralSceneWatcher(const shared_ptr<Node>& node);
  virtual ~GeneralSceneWatcher();

  /// Initializes resources needed for scene watchers
  static void Init();

  /// Called when the watcher needs to be rerendered
  virtual void OnRedraw() override;

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(EventForwarderGLWidget* widget);

  RenderTarget* mRenderTarget = nullptr;

  /// Scene node to be drawn.
  shared_ptr<SceneNode> mScene;
  
  /// Default scene node, might be unused
  shared_ptr<SceneNode> mDefaultScene = make_shared<SceneNode>();

  /// Default camera
  shared_ptr<CameraNode> mCamera = make_shared<CameraNode>();

  /// Forwarder that catches render events for the default scene
  shared_ptr<RenderForwarder> mRenderForwarder;

  /// Qt widget event handlers
  void HandleMousePress(EventForwarderGLWidget*, QMouseEvent* event);
  void HandleMouseRelease(EventForwarderGLWidget*, QMouseEvent* event);
  void HandleMouseMove(EventForwarderGLWidget*, QMouseEvent* event);
  void HandleMouseWheel(EventForwarderGLWidget*, QWheelEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(EventForwarderGLWidget*, QKeyEvent* event);

  /// Mouse position at pressing
  QPoint mOriginalPosition;
  Vec3 mOriginalOrientation;
  float mOriginalDistance;

  /// Static resources initializes by Init()
  static shared_ptr<Material> mDefaultMaterial;

private:
  /// The Drawable supplied by an implementation
  shared_ptr<Drawable> mDrawable;

  Globals mGlobals;
};


