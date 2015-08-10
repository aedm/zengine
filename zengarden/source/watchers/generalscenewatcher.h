#pragma once

#include "watcher.h"
#include "watcherwidget.h"
#include <zengine.h>

class GeneralSceneWatcher: public Watcher {
public:
  GeneralSceneWatcher(Node* node, GLWatcherWidget* watcherWidget);
  virtual ~GeneralSceneWatcher();

  /// Initializes resources needed for scene watchers
  static void Init();

protected:
  void Paint(GLWidget* widget);
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  /// Scene node to be drawn.
  SceneNode* mScene;
  
  /// Default scene node, might be unused
  SceneNode mDefaultScene;

  /// Default camera
  Camera mCamera;

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