#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>

enum class WatcherPosition 
{
	LEFT_TAB,
	RIGHT_TAB,
};

class WatcherWidget : public QWidget
{
public:
	WatcherWidget(QWidget* Parent, WatcherPosition Position, bool CanBeClosed);
	~WatcherWidget();

	Node*							Watcher;

	/// Triggered when a new node needs to be watched
	Event<Node*, WatcherWidget*>	OnWatchNode;

	WatcherPosition					Position;
	bool							CanBeClosed;
};