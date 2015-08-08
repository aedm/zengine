#pragma once

#include "../dom/node.h"
#include "../shaders/stubnode.h"

class Camera: public Node {
public:
  Camera();
  virtual ~Camera();

  void SetupGlobals(Globals* globals, const Vec2& canvasSize);

  /// Camera setup
  float mFovY = 60.0f * (Pi / 180.0f);
  float mZFar = 150.0f;
  float mZNear = 0.01f;
  Vec3 mTarget = Vec3(0, 0, 0);
  float mDistance = 100;
  Vec3 mOrientation = Vec3(0, 0, 0);

  bool mOrthonormal = false;
};

typedef TypedSlot<NodeType::CAMERA, Camera> CameraSlot;
