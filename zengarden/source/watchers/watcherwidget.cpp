#include "watcherwidget.h"

WatcherWidget::WatcherWidget(QWidget* Parent, WatcherPosition _Position, bool _CanBeClosed)
	: QWidget(Parent)
	, Position(_Position)
	, Watcher(nullptr)
	, CanBeClosed(_CanBeClosed)
{
}

WatcherWidget::~WatcherWidget()
{
	SafeDelete(Watcher);
}


