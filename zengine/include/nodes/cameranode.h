#pragma once

#include "../dom/node.h"
#include "../shaders/stubnode.h"

class CameraNode: public Node {
public:
  CameraNode();
  virtual ~CameraNode();

  void SetupGlobals(Globals* globals) const;

  /// Camera setup
  FloatSlot mFovY;
  FloatSlot mZFar;
  FloatSlot mZNear;
  FloatSlot mDistance;
  Vec3Slot mTarget;
  Vec3Slot mOrientation;

  FloatSlot mShake;
  FloatSlot mShakeTime;
  FloatSlot mShakeSpeed;
  
  bool mOrthonormal = false;

protected:
  /// Handle received messages
  void HandleMessage(Message* message) override;
};

typedef TypedSlot<CameraNode> CameraSlot;
