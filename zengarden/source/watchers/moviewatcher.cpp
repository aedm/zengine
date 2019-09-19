#include "moviewatcher.h"
#include "../zengarden.h"

MovieWatcher::MovieWatcher(const shared_ptr<Node>& node)
  : WatcherUI(node)
{
  ZenGarden::GetInstance()->mOnMovieCursorChange += 
    Delegate(this, &MovieWatcher::HandleMovieCursorChange);
}

MovieWatcher::~MovieWatcher() {
  ZenGarden::GetInstance()->mOnMovieCursorChange -= 
    Delegate(this, &MovieWatcher::HandleMovieCursorChange);
}

void MovieWatcher::OnRedraw() {
  GetGLWidget()->update();
}

void MovieWatcher::OnTimeEdited(float time) {
  ZenGarden::GetInstance()->SetMovieCursor(time);
}

void MovieWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  GetGLWidget()->OnPaint += Delegate(this, &MovieWatcher::Paint);
}

void MovieWatcher::Paint(EventForwarderGLWidget* widget) {
  if (!mWatcherWidget) return;

  shared_ptr<MovieNode> movieNode = PointerCast<MovieNode>(GetNode());
  if (!movieNode) return;

  if (!mRenderTarget) {
    GetGLWidget()->makeCurrent();
    mRenderTarget =
      new RenderTarget(Vec2(float(mWatcherWidget->width()), float(mWatcherWidget->height())));
  }

  const Vec2 size = Vec2(widget->width(), widget->height());
  mRenderTarget->Resize(size);

  movieNode->Draw(mRenderTarget, ZenGarden::GetInstance()->GetMovieCursor());
}

void MovieWatcher::HandleMovieCursorChange(float movieCursor) {
  GetGLWidget()->update();
}
