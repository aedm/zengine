#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

/// Widget that displays node parameters
class PropertyEditor : public Node
{
public:
	PropertyEditor(Node* Nd, QWidget* PropertyPanel);
	virtual ~PropertyEditor();

protected:
	QWidget*				PropertyPanel;
	QBoxLayout*				Layout;

	Slot					WatchedNode;

	
};