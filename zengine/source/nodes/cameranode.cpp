#include <include/nodes/cameranode.h>
#include <glm/gtc/matrix_transform.hpp>

REGISTER_NODECLASS(CameraNode, "Camera");

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
  globals->World = mat4x4(); /// Load identity matrix

  if (mOrthonormal) {
    globals->View = mat4x4();
    globals->Camera = mat4x4();
    globals->Projection = Matrix::Ortho(0, 0, canvasSize.x, canvasSize.y);
    return;
  }

  /// Projection matrix
  const float aspectRatio = canvasSize.x / canvasSize.y;
  globals->Projection = 
    glm::perspective(mFovY.Get(), aspectRatio, mZNear.Get(), mZFar.Get());

  /// Camera matrix
  //Matrix rotate = 
  //  Matrix::Rotate(Quaternion::FromEuler(mOrientation.x, mOrientation.y, 0));
  const Matrix xRot = Matrix::Rotate(mOrientation.Get().x, vec3(1, 0, 0));
  const Matrix yRot = Matrix::Rotate(mOrientation.Get().y, vec3(0, 1, 0));
  //Matrix lookAt = 
  //  Matrix::LookAt(vec3(0, 0, mDistance.Get()), mTarget.Get(), vec3(0, 1, 0));
  //globals->View = lookAt * xRot * yRot;
  const Matrix target = Matrix::Translate(-mTarget.Get());
  const Matrix distance = Matrix::Translate(vec3(0, 0, -mDistance.Get()));
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
    const Matrix xShakeRot = Matrix::Rotate(xAngle, vec3(1, 0, 0));
    const Matrix yShakeRot = Matrix::Rotate(yAngle, vec3(0, 1, 0));
    const Matrix zRot = Matrix::Rotate(zAngle, vec3(0, 0, 1));
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
