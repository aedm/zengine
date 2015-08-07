#include "generalscenewatcher.h"
#include "../util/util.h"

Material* GeneralSceneWatcher::mDefaultMaterial = nullptr;

GeneralSceneWatcher::GeneralSceneWatcher(Node* node, GLWatcherWidget* watcherWidget) 
  : Watcher(node, watcherWidget)
{
  GetGLWidget()->OnPaint += Delegate(this, &GeneralSceneWatcher::Paint);
  GetGLWidget()->OnMousePress += Delegate(this, &GeneralSceneWatcher::HandleMousePress);
  GetGLWidget()->OnMouseRelease += Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  GetGLWidget()->OnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);
}

GeneralSceneWatcher::~GeneralSceneWatcher() {
  SafeDelete(mDrawable);
}

void GeneralSceneWatcher::Paint(GLWidget* widget) {
  ASSERT(mDrawable);

  TheDrawingAPI->Clear(true, true, 0x202020);

  Vec2 size = Vec2(widget->width(), widget->height());
  mGlobals.RenderTargetSize = size;
  mGlobals.RenderTargetSizeRecip = Vec2(1.0f / size.x, 1.0f / size.y);

  if (mOrthonormal) {
    mGlobals.View.LoadIdentity();
    mGlobals.Projection = Matrix::Ortho(0, 0, size.x, size.y);
  }
  else {
    float aspectRatio = size.x / size.y;
    mGlobals.Projection = Matrix::Projection(mFovY, mZFar, mZNear, aspectRatio);
    //Matrix rotate = 
    //  Matrix::Rotate(Quaternion::FromEuler(mOrientation.x, mOrientation.y, 0));
    Matrix xRot = Matrix::Rotate(mOrientation.x, Vec3(1, 0, 0));
    Matrix yRot = Matrix::Rotate(mOrientation.y, Vec3(0, 1, 0));
    Matrix lookAt = Matrix::LookAt(Vec3(0, 0, 20), mTarget, Vec3(0, 1, 0));
    mGlobals.View = lookAt * xRot * yRot;
  }
  
  mGlobals.Transformation = mGlobals.Projection * mGlobals.View;

  Vec4 t = Vec4(10, 10, 10, 1) * mGlobals.Transformation;
  Vec3 tt = Vec3(t.x / t.w, t.y / t.w, t.z / t.w);

  mDrawable->Draw(&mGlobals);
}

void GeneralSceneWatcher::HandleSniffedMessage(NodeMessage message, Slot* slot, 
                                               void* payload) 
{
  if (message == NodeMessage::NEEDS_REDRAW) {
    GetGLWidget()->updateGL();
  }
}

void GeneralSceneWatcher::Init()
{
  StubNode* defaultVertex = Util::LoadStub("engine/scenewatcher/defaultvertex.shader");
  StubNode* defaultFragment = Util::LoadStub("engine/scenewatcher/defaultfragment.shader");
  
  Pass* defaultPass = new Pass();
  defaultPass->mFragmentStub.Connect(defaultFragment);
  defaultPass->mVertexStub.Connect(defaultVertex);

  mDefaultMaterial = new Material();
  mDefaultMaterial->mSolidPass.Connect(defaultPass);
}

void GeneralSceneWatcher::HandleMousePress(GLWidget*, QMouseEvent* event) {
  mOriginalPosition = event->pos();
  mOriginalOrientation = mOrientation;
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}

void GeneralSceneWatcher::HandleMouseRelease(GLWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}

void GeneralSceneWatcher::HandleMouseMove(GLWidget*, QMouseEvent* event) {
  if (event->buttons() & Qt::LeftButton) {
    auto diff = event->pos() - mOriginalPosition;
    mOrientation.y = mOriginalOrientation.y - float(diff.x()) / 300.0f;
    mOrientation.x = mOriginalOrientation.x - float(diff.y()) / 300.0f;
    GetGLWidget()->update();
  }
}

void GeneralSceneWatcher::HandleMouseWheel(GLWidget*, QWheelEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseLeftUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightDown(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleMouseRightUp(QMouseEvent* event) {

}

void GeneralSceneWatcher::HandleKeyPress(GLWidget*, QKeyEvent* event) {

}

