#include <include/nodes/cameranode.h>

REGISTER_NODECLASS(CameraNode, "Camera");

static SharedString FovYSlotName = make_shared<string>("Field of view");
static SharedString ZNearSlotName = make_shared<string>("Near Z");
static SharedString ZFarSlotName = make_shared<string>("Far Z");
static SharedString DistanceSlotName = make_shared<string>("Distance");
static SharedString TargetSlotName = make_shared<string>("Target");
static SharedString OrientationSlotName = make_shared<string>("Orientation");
static SharedString ShakeSlotName = make_shared<string>("Shake");
static SharedString ShakeTimeSlotName = make_shared<string>("ShakeTime");
static SharedString ShakeSpeedSlotName = make_shared<string>("ShakeSpeed");

CameraNode::CameraNode() 
  : Node(NodeType::CAMERA)
  , mTarget(this, TargetSlotName)
  , mDistance(this, DistanceSlotName)
  , mOrientation(this, OrientationSlotName)
  , mFovY(this, FovYSlotName)
  , mZNear(this, ZNearSlotName)
  , mZFar(this, ZFarSlotName)
  , mShake(this, ShakeSlotName)
  , mShakeTime(this, ShakeTimeSlotName)
  , mShakeSpeed(this, ShakeSpeedSlotName)
{
  mFovY.SetDefaultValue(60.0f * (Pi / 180.0f));
  mZNear.SetDefaultValue(1.0f);
  mZFar.SetDefaultValue(1000.0f);
  mTarget.SetDefaultValue(Vec3(0, 0, 0));
  mDistance.SetDefaultValue(50.0f);
  mOrientation.SetDefaultValue(Vec3(0, 0, 0));
}

void CameraNode::SetupGlobals(Globals* globals) {
  Vec2 canvasSize = globals->RenderTargetSize;

  if (mOrthonormal) {
    globals->View.LoadIdentity();
    globals->Projection = Matrix::Ortho(0, 0, canvasSize.x, canvasSize.y);
  } else {
    float aspectRatio = canvasSize.x / canvasSize.y;
    globals->Projection = 
      Matrix::Projection(mFovY.Get(), mZFar.Get(), mZNear.Get(), aspectRatio);
    //Matrix rotate = 
    //  Matrix::Rotate(Quaternion::FromEuler(mOrientation.x, mOrientation.y, 0));
    Matrix xRot = Matrix::Rotate(mOrientation.Get().x, Vec3(1, 0, 0));
    Matrix yRot = Matrix::Rotate(mOrientation.Get().y, Vec3(0, 1, 0));
    //Matrix lookAt = 
    //  Matrix::LookAt(Vec3(0, 0, mDistance.Get()), mTarget.Get(), Vec3(0, 1, 0));
    //globals->View = lookAt * xRot * yRot;
    Matrix target = Matrix::Translate(-mTarget.Get());
    Matrix distance = Matrix::Translate(Vec3(0, 0, -mDistance.Get()));
    globals->View = distance * xRot * yRot * target;

    float shake = mShake.Get() * 0.1f;
    if (shake > 0.0f) {
      float time = mShakeTime.Get() * mShakeSpeed.Get();
      float xAngle = 
        (sinf(time) + cosf(time * 2.53f) + sinf(time * 3.91f + 0.3f)) * shake;
      float yAngle =
        (sinf(time * 0.87f) + cosf(time * 2.23f) + cosf(time * 3.71f + 0.8f)) * shake;
      float zAngle =
        (sinf(time * 0.67f) + cosf(time * 2.43f) + cosf(time * 3.81f + 0.5f)) * shake;
      Matrix xRot = Matrix::Rotate(xAngle, Vec3(1, 0, 0));
      Matrix yRot = Matrix::Rotate(yAngle, Vec3(0, 1, 0));
      Matrix zRot = Matrix::Rotate(zAngle, Vec3(0, 0, 1));
      globals->View = xRot * yRot * zRot * globals->View;
    }
  }

  globals->Transformation = globals->Projection * globals->View;
}

CameraNode::~CameraNode() {
}

void CameraNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
      SendMsg(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}
