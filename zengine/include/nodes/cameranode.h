#pragma once

#include "../dom/node.h"
#include "../shaders/stubnode.h"

class CameraNode: public Node {
public:
  CameraNode();
  virtual ~CameraNode();

  void SetupGlobals(Globals* globals, const Vec2& canvasSize);

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
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};

typedef TypedSlot<NodeType::CAMERA, CameraNode> CameraSlot;
