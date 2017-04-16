#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include <zengine.h>

class SceneNode;

class RenderForwarder: public Watcher {
public:
  RenderForwarder(SceneNode* node);
  virtual ~RenderForwarder() {}
  virtual void OnRedraw();
  FastDelegate<void()> mOnRedraw;
};

class GeneralSceneWatcher: public WatcherUI {
public:
  GeneralSceneWatcher(Node* node);
  virtual ~GeneralSceneWatcher();

  /// Initializes resources needed for scene watchers
  static void Init();

  /// Called when the watcher needs to be rerendered
  virtual void OnRedraw() override;

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  void Paint(GLWidget* widget);

  RenderTarget* mRenderTarget = nullptr;

  /// Scene node to be drawn.
  SceneNode* mScene = nullptr;
  
  /// Default scene node, might be unused
  SceneNode mDefaultScene;

  /// Default camera
  CameraNode mCamera;

  /// Forwarder that catches render events for the default scene
  shared_ptr<RenderForwarder> mRenderForwarder;

  /// Qt widget event handlers
  void HandleMousePress(GLWidget*, QMouseEvent* event);
  void HandleMouseRelease(GLWidget*, QMouseEvent* event);
  void HandleMouseMove(GLWidget*, QMouseEvent* event);
  void HandleMouseWheel(GLWidget*, QWheelEvent* event);
  void HandleMouseLeftDown(QMouseEvent* event);
  void HandleMouseLeftUp(QMouseEvent* event);
  void HandleMouseRightDown(QMouseEvent* event);
  void HandleMouseRightUp(QMouseEvent* event);
  void HandleKeyPress(GLWidget*, QKeyEvent* event);

  /// Mouse position at pressing
  QPoint mOriginalPosition;
  Vec3 mOriginalOrientation;
  float mOriginalDistance;

  /// Static resources initializes by Init()
  static Material* mDefaultMaterial;

private:
  /// The Drawable supplied by an implementation
  Drawable* mDrawable = nullptr;
};


