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
	virtual void HandleSniffedMessage(Slot* slot, NodeMessage message, 
                                    void* payload) override;

	QListView* mListView;
	QStandardItemModel* mModel;
};