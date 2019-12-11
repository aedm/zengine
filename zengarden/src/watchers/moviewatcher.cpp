#include "moviewatcher.h"
#include "../zengarden.h"

MovieWatcher::MovieWatcher(const std::shared_ptr<Node>& node)
  : WatcherUi(node)
{
  ZenGarden::GetInstance()->mOnMovieCursorChange += 
    Delegate(this, &MovieWatcher::HandleMovieCursorChange);
}

MovieWatcher::~MovieWatcher() {
  ZenGarden::GetInstance()->mOnMovieCursorChange -= 
    Delegate(this, &MovieWatcher::HandleMovieCursorChange);
}

void MovieWatcher::OnRedraw() {
  GetGlWidget()->update();
}

void MovieWatcher::OnTimeEdited(float time) {
  ZenGarden::GetInstance()->SetMovieCursor(time);
}

void MovieWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);
  GetGlWidget()->mOnPaint += Delegate(this, &MovieWatcher::Paint);
}

void MovieWatcher::Paint(EventForwarderGlWidget* widget) {
  if (!mWatcherWidget) return;

  std::shared_ptr<MovieNode> movieNode = PointerCast<MovieNode>(GetNode());
  if (!movieNode) return;

  if (!mRenderTarget) {
    GetGlWidget()->makeCurrent();
    mRenderTarget =
      new RenderTarget(ivec2(mWatcherWidget->width(), mWatcherWidget->height()));
  }
  mRenderTarget->Resize(ivec2(widget->width(), widget->height()));
  movieNode->Draw(mRenderTarget, ZenGarden::GetInstance()->GetMovieCursor());
}

void MovieWatcher::HandleMovieCursorChange(float movieCursor) const
{
  GetGlWidget()->update();
}
