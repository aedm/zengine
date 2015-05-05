#pragma once

#include <zengine.h>
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>

/// Widget that displays node parameters
class PropertyEditor : public Node
{
public:
	virtual ~PropertyEditor();

	static PropertyEditor*	ForNode(Node* Nd, QWidget* PropertyPanel);

protected:
	PropertyEditor(Node* Nd, QWidget* PropertyPanel);

	QWidget*				PropertyPanel;
	QBoxLayout*				Layout;

	Slot					WatchedNode;

	
};