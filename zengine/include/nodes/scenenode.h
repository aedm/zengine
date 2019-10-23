#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "cameranode.h"
#include "../render/rendertarget.h"
#include "../shaders/pass.h"
#include "timenode.h"
#include <memory>

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
  FloatSlot mSkyLightBias;
  FloatSlot mDOFEnabled;
  FloatSlot mDOFFocusDistance;
  FloatSlot mDOFBlur;
  FloatSlot mDOFScale;
  FloatSlot mDOFBleed;
  FloatSlot mZPrepassDisabled;
  FluidSlot mFluidsSlot;

  void Draw(RenderTarget* renderTarget, Globals* globals);

  /// Sets the clip-relative time to all SceneTimeNode dependencies
  void SetSceneTime(float beats);

  /// Gets the current time cursor
  float GetSceneTime() const;

  /// hack
  void UpdateDependencies();

protected:
  void Operate() override;

  void RenderDrawables(Globals* globals, PassType passType) const;

  /// Handle received messages
  void HandleMessage(Message* message) override;

private:
  std::vector<std::shared_ptr<Node>> mTransitiveClosure;
  Slot mSceneTimes;
  float mSceneTime = 0.0f;
  float mLastRenderTime{};

  std::shared_ptr<GlobalTimeNode> mGlobalTimeNode = std::make_shared<GlobalTimeNode>();

  void CalculateRenderDependencies();
};

typedef TypedSlot<SceneNode> SceneSlot;
