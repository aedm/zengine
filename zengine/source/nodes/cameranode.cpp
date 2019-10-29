#include <include/nodes/cameranode.h>
#include <glm/gtc/matrix_transform.hpp>

REGISTER_NODECLASS(CameraNode, "Camera");

using glm::ivec2;

CameraNode::CameraNode()
  : mTarget(this, "Target", false, true, true, 0.0f, 10.0f)
  , mDistance(this, "Distance", false, true, true, 0.0f, 10.0f)
  , mOrientation(this, "Orientation", false, true, true, -Pi, Pi)
  , mFovY(this, "Field of view")
  , mZNear(this, "Near Z")
  , mZFar(this, "Far Z")
  , mShake(this, "Shake")
  , mShakeTime(this, "ShakeTime")
  , mShakeSpeed(this, "ShakeSpeed") {
  mFovY.SetDefaultValue(60.0f * (Pi / 180.0f));
  mZNear.SetDefaultValue(0.1f);
  mZFar.SetDefaultValue(200.0f);
  mTarget.SetDefaultValue(vec3(0, 0, 0));
  mDistance.SetDefaultValue(5.0f);
  mOrientation.SetDefaultValue(vec3(0, 0, 0));
}

void CameraNode::SetupGlobals(Globals* globals) const
{
  const vec2 canvasSize = globals->RenderTargetSize;
  globals->World = mat4(); /// Load identity matrix

  if (mOrthonormal) {
    globals->View = mat4();
    globals->Camera = mat4();
    globals->Projection = glm::ortho(0.0f, 0.0f, canvasSize.x, canvasSize.y);
    return;
  }

  /// Projection matrix
  const float aspectRatio = canvasSize.x / canvasSize.y;
  globals->Projection = 
    glm::perspective(mFovY.Get(), aspectRatio, mZNear.Get(), mZFar.Get());

  /// Camera matrix
  //Matrix rotate = 
  //  Matrix::Rotate(Quaternion::FromEuler(mOrientation.x, mOrientation.y, 0));
  const mat4 xRot = glm::rotate(mat4(), mOrientation.Get().x, vec3(1, 0, 0));
  const mat4 yRot = glm::rotate(mat4(), mOrientation.Get().y, vec3(0, 1, 0));
  //Matrix lookAt = 
  //  Matrix::LookAt(vec3(0, 0, mDistance.Get()), mTarget.Get(), vec3(0, 1, 0));
  //globals->View = lookAt * xRot * yRot;
  const mat4 target = glm::translate(mat4(), -mTarget.Get());
  const mat4 distance = glm::translate(mat4(), { 0, 0, -mDistance.Get() });
  globals->Camera = distance * xRot * yRot * target;

  const float shake = mShake.Get() * 0.1f;
  if (shake > 0.0f) {
    const float time = mShakeTime.Get() * mShakeSpeed.Get();
    const float xAngle =
      (sinf(time) + cosf(time * 2.53f) + sinf(time * 3.91f + 0.3f)) * shake;
    const float yAngle =
      (sinf(time * 0.87f) + cosf(time * 2.23f) + cosf(time * 3.71f + 0.8f)) * shake;
    const float zAngle =
      (sinf(time * 0.67f) + cosf(time * 2.43f) + cosf(time * 3.81f + 0.5f)) * shake;
    const mat4 xShakeRot = glm::rotate(mat4(), xAngle, { 1, 0, 0 });
    const mat4 yShakeRot = glm::rotate(mat4(), yAngle, { 0, 1, 0 });
    const mat4 zRot = glm::rotate(mat4(), zAngle, { 0, 0, 1 });
    globals->Camera = xShakeRot * yShakeRot * zRot * globals->Camera;
  }
}

CameraNode::~CameraNode() = default;

void CameraNode::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
    case MessageType::VALUE_CHANGED:
      SendMsg(MessageType::NEEDS_REDRAW);
      break;
    default: break;
  }
}
