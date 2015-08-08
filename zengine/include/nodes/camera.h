#pragma once

#include "../dom/node.h"
#include "../shaders/stubnode.h"

class Camera: public Node {
public:
  Camera();
  virtual ~Camera();

  void SetupGlobals(Globals* globals, const Vec2& canvasSize);

  /// Camera setup
  FloatSlot mFovY;
  FloatSlot mZFar;
  FloatSlot mZNear;
  FloatSlot mDistance;
  Vec3Slot mTarget;
  Vec3Slot mOrientation;

  bool mOrthonormal = false;
};

typedef TypedSlot<NodeType::CAMERA, Camera> CameraSlot;
