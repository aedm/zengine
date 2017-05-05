#include "moviewatcher.h"

MovieWatcher::MovieWatcher(Node* node)
  : WatcherUI(node)
{

}

MovieWatcher::~MovieWatcher() {

}

void MovieWatcher::OnRedraw() {
  GetGLWidget()->update();
}

void MovieWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  GetGLWidget()->OnPaint += Delegate(this, &MovieWatcher::Paint);
  //GetGLWidget()->OnMousePress += Delegate(this, &GeneralSceneWatcher::HandleMousePress);
  //GetGLWidget()->OnMouseRelease += Delegate(this, &GeneralSceneWatcher::HandleMouseRelease);
  //GetGLWidget()->OnMouseMove += Delegate(this, &GeneralSceneWatcher::HandleMouseMove);
  //GetGLWidget()->OnKeyPress += Delegate(this, &GeneralSceneWatcher::HandleKeyPress);
  //GetGLWidget()->OnMouseWheel += Delegate(this, &GeneralSceneWatcher::HandleMouseWheel);

}

void MovieWatcher::Paint(GLWidget* widget) {
  if (!mWatcherWidget) return;

  MovieNode* movieNode = dynamic_cast<MovieNode*>(mNode);
  if (!movieNode) return;

  if (!mRenderTarget) {
    GetGLWidget()->makeCurrent();
    mRenderTarget =
      new RenderTarget(Vec2(float(mWatcherWidget->width()), float(mWatcherWidget->height())));
  }

  Vec2 size = Vec2(widget->width(), widget->height());
  mRenderTarget->Resize(size);
  movieNode->Draw(mRenderTarget);
}
