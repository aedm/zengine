#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_documentviewer.h"
#include <zengine.h>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public WatcherUI {
public:
  DocumentWatcher(const shared_ptr<Node>& documentNode);
  virtual ~DocumentWatcher();

  void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  Ui::DocumentViewer mUI;

  void OnSlotConnectionChanged(Slot* slot) override;
  void OnChildNameChange() override;

  void RefreshGraphList() const;

  QStandardItemModel* mModel = nullptr;
};