#pragma once

#include "watcherui.h"
#include "ui_documentviewer.h"
#include <zengine.h>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public WatcherUi {
public:
  DocumentWatcher(const shared_ptr<Node>& documentNode);
  virtual ~DocumentWatcher();

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  Ui::DocumentViewer mUi {};

  void OnSlotConnectionChanged(Slot* slot) override;
  void OnChildNameChange() override;

  void RefreshGraphList() const;

  QStandardItemModel* mModel = nullptr;
};