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

  /// Static resources initializes by Init()
  static Material* mDefaultMaterial;
};