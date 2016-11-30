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
  virtual void OnMultiSlotConnectionAdded(Slot* slot, Node* addedNode) override;
  virtual void OnMultiSlotConnectionRemoved(Slot* slot, Node* removedNode) override;

  void RefreshGraphList();

	QListView* mListView;
	QStandardItemModel* mModel;
};