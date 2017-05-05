#include "timelineeditor.h"
#include "../zengarden.h"

TimelineEditor::TimelineEditor(MovieNode* movieNode)
  : WatcherUI(movieNode)
{

}

TimelineEditor::~TimelineEditor() {
}

void TimelineEditor::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUI::SetWatcherWidget(watcherWidget);
  mUI.setupUi(watcherWidget);

  watcherWidget->connect(mUI.newClipButton, &QPushButton::pressed, [=]() {
    ClipNode* clipNode = new ClipNode();
    MovieNode* movieNode = dynamic_cast<MovieNode*>(this->mNode);
    movieNode->mClips.Connect(clipNode);
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(clipNode);
  });
}
