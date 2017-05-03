#include "moviewatcher.h"

MovieWatcher::MovieWatcher(MovieNode* movieNode)
  : WatcherUI(movieNode)
{

}

MovieWatcher::~MovieWatcher() {
}

void MovieWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  mUI.setupUi(watcherWidget);
}
