#pragma once

#include "../dom/node.h"
#include "../shaders/stubnode.h"

class CameraNode: public Node {
public:
  CameraNode();
  virtual ~CameraNode();

  void SetupGlobals(Globals* globals) const;

  /// Camera setup
  Vec3Slot mTarget;
  FloatSlot mDistance;
  Vec3Slot mOrientation;
  FloatSlot mFovY;
  FloatSlot mZNear;
  FloatSlot mZFar;

  FloatSlot mShake;
  FloatSlot mShakeTime;
  FloatSlot mShakeSpeed;
  
  bool mOrthonormal = false;

protected:
  /// Handle received messages
  void HandleMessage(Message* message) override;
};

typedef TypedSlot<CameraNode> CameraSlot;
