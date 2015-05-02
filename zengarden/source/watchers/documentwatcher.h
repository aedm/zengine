#pragma once

#include "../document.h"
#include <zengine.h>
#include <QtWidgets/QListView>
#include <QtGui/QStandardItemModel>

class DocumentWatcher : public Node
{
public:
	DocumentWatcher(QListView* ListView, Document* DocumentNode);
	virtual ~DocumentWatcher();

protected:
	virtual void		HandleMessage(Slot* S, NodeMessage Message);

	Slot				DocumentSlot;
	QListView*			ListView;
	QStandardItemModel* Model;
};