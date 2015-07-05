#pragma once

#include "watcher.h"
#include <zengine.h>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public Watcher
{
public:
	DocumentWatcher(QListView* ListView, Document* DocumentNode);
	virtual ~DocumentWatcher();

protected:
	virtual void		HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload);

	QListView*			ListView;
	QStandardItemModel* Model;
};