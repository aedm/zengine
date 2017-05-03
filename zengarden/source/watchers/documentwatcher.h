#pragma once

#include "watcherui.h"
#include "watcherwidget.h"
#include "ui_documentviewer.h"
#include <zengine.h>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public WatcherUI
{
public:
	DocumentWatcher(Document* documentNode);
	virtual ~DocumentWatcher();

  virtual void SetWatcherWidget(WatcherWidget* watcherWidget) override;

protected:
  Ui::DocumentViewer mUI;

  virtual void OnSlotConnectionChanged(Slot* slot);

  void RefreshGraphList();

	QStandardItemModel* mModel = nullptr;
};