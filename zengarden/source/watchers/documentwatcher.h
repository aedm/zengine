#pragma once

#include "watcher.h"
#include <zengine.h>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public Watcher
{
public:
	DocumentWatcher(QListView* listView, Document* documentNode);
	virtual ~DocumentWatcher();

protected:
  virtual void HandleSniffedMessage(NodeMessage message, Slot* slot,
                                    void* payload) override;

  void RefreshGraphList();

	QListView* mListView;
	QStandardItemModel* mModel;
};