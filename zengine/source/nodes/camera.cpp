#include <include/nodes/camera.h>

REGISTER_NODECLASS(Camera, "Camera");

static SharedString FovYSlotName = make_shared<string>("Field of view");
static SharedString ZNearSlotName = make_shared<string>("Near Z");
static SharedString ZFarSlotName = make_shared<string>("Far Z");
static SharedString DistanceSlotName = make_shared<string>("Distance");
static SharedString TargetSlotName = make_shared<string>("Target");
static SharedString OrientationSlotName = make_shared<string>("Orientation");
//static SharedString SlotName = make_shared<string>("");
//static SharedString SlotName = make_shared<string>("");
//static SharedString SlotName = make_shared<string>("");
//static SharedString SlotName = make_shared<string>("");

Camera::Camera() 
  : Node(NodeType::CAMERA)
  , mTarget(this, TargetSlotName)
  , mDistance(this, DistanceSlotName)
  , mOrientation(this, OrientationSlotName)
  , mFovY(this, FovYSlotName)
  , mZNear(this, ZNearSlotName)
  , mZFar(this, ZFarSlotName)
{
  mFovY.SetDefaultValue(60.0f * (Pi / 180.0f));
  mZNear.SetDefaultValue(0.1f);
  mZFar.SetDefaultValue(150.0f);
  mTarget.SetDefaultValue(Vec3(0, 0, 0));
  mDistance.SetDefaultValue(50.0f);
  mOrientation.SetDefaultValue(Vec3(0, 0, 0));
}

void Camera::SetupGlobals(Globals* globals, const Vec2& canvasSize) {
  globals->RenderTargetSize = canvasSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / canvasSize.x, 1.0f / canvasSize.y);

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
    Matrix lookAt = 
      Matrix::LookAt(Vec3(0, 0, mDistance.Get()), mTarget.Get(), Vec3(0, 1, 0));
    globals->View = lookAt * xRot * yRot;
  }

  globals->Transformation = globals->Projection * globals->View;
}

Camera::~Camera() {
}
