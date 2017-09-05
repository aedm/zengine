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
  Vec3Slot mSkyLightColor;
  FloatSlot mSkyLightAmbient;
  FloatSlot mSkyLightSpread;
  FloatSlot mSkyLightSampleSpread;

  void Draw(RenderTarget* renderTarget, Globals* globals);

  /// Sets the clip-relative time to all SceneTimeNode dependencies
  void SetSceneTime(float seconds);

  /// Gets the currect time cursor
  float GetSceneTime();

protected:
  void RenderDrawables(Globals* globals, PassType passType);

  /// Handle received messages
  virtual void HandleMessage(Message* message) override;

private:
  vector<Node*> mTransitiveClosure;
  Slot mSceneTimes;
  float mSceneTime = 0.0f;

  void CalculateRenderDependencies();
};

typedef TypedSlot<NodeType::SCENE, SceneNode> SceneSlot;
