#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "cameranode.h"
#include "../render/rendertarget.h"
#include "../shaders/pass.h"

/// Local time of the current scene
/// TODO: make it not global somehow
extern FloatNode* TheSceneTime;

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;
  CameraSlot mCamera;

  Vec3Slot mShadowMapSize;
  Vec3Slot mSkyLightDirection;

  void Draw(RenderTarget* renderTarget);

protected:
  Globals mGlobals;

  void RenderDrawables(PassType passType);

  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};

typedef TypedSlot<NodeType::SCENE, SceneNode> SceneSlot;
