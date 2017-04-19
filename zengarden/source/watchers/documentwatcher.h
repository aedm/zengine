#pragma once

#include "watcherui.h"
#include <zengine.h>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public WatcherUI
{
public:
	DocumentWatcher(QListView* listView, Document* documentNode);
	virtual ~DocumentWatcher();

protected:
  virtual void OnSlotConnectionChanged(Slot* slot);

  void RefreshGraphList();

	QListView* mListView;
	QStandardItemModel* mModel;
};