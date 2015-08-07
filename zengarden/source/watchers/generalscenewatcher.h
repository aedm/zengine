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

  Drawable* mDrawable = nullptr;
  Globals mGlobals;

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

  /// Camera setup
  float mFovY = 60.0f * (Pi / 180.0f);
  float mZFar = 150.0f;
  float mZNear = 0.01f;
  Vec3 mTarget = Vec3(0, 0, 0);
  float mDistance = 100;
  Vec3 mOrientation = Vec3(0, 0, 0);
    
  bool mOrthonormal = false;

  /// Mouse position at pressing
  QPoint mOriginalPosition;
  Vec3 mOriginalOrientation;

  /// Static resources initializes by Init()
  static Material* mDefaultMaterial;
};