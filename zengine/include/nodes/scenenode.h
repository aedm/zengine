#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "cameranode.h"
#include "../render/rendertarget.h"
#include "../shaders/pass.h"
#include "timenode.h"

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;
  CameraSlot mCamera;

  Vec3Slot mShadowMapSize;
  Vec3Slot mSkyLightDirection;

  void Draw(RenderTarget* renderTarget, Globals* globals);

  /// Sets the clip-relative time to all SceneTimeNode dependencies
  void SetSceneTime(float seconds);

protected:
  void RenderDrawables(Globals* globals, PassType passType);

  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot) override;

private:
  vector<Node*> mTransitiveClosure;
  vector<SceneTimeNode*> mDependentSceneTimeNodes;

  void CalculateRenderDependencies();
};

typedef TypedSlot<NodeType::SCENE, SceneNode> SceneSlot;
