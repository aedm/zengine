#include <include/nodes/camera.h>

REGISTER_NODECLASS(Camera, "Camera");

Camera::Camera() 
  : Node(NodeType::CAMERA)
{

}

void Camera::SetupGlobals(Globals* globals, const Vec2& canvasSize) {
  globals->RenderTargetSize = canvasSize;
  globals->RenderTargetSizeRecip = Vec2(1.0f / canvasSize.x, 1.0f / canvasSize.y);

  if (mOrthonormal) {
    globals->View.LoadIdentity();
    globals->Projection = Matrix::Ortho(0, 0, canvasSize.x, canvasSize.y);
  } else {
    float aspectRatio = canvasSize.x / canvasSize.y;
    globals->Projection = Matrix::Projection(mFovY, mZFar, mZNear, aspectRatio);
    //Matrix rotate = 
    //  Matrix::Rotate(Quaternion::FromEuler(mOrientation.x, mOrientation.y, 0));
    Matrix xRot = Matrix::Rotate(mOrientation.x, Vec3(1, 0, 0));
    Matrix yRot = Matrix::Rotate(mOrientation.y, Vec3(0, 1, 0));
    Matrix lookAt = Matrix::LookAt(Vec3(0, 0, 20), mTarget, Vec3(0, 1, 0));
    globals->View = lookAt * xRot * yRot;
  }

  globals->Transformation = globals->Projection * globals->View;
}

Camera::~Camera() {
}
