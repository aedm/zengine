#pragma once

#include <zengine.h>
#include <QFrame>
#include <QGLWidget>
#include <vector>

using namespace std;
class NodeWidget;
class GraphNode;

class GraphEditor: public QGLWidget { Q_OBJECT
	friend class CreateNodeCommand;
	friend class NodeWidget;

public:
	GraphEditor(QWidget* Parent, QGLWidget* Share);	

	NodeWidget*						GetNodeWidget(Node* Nd);
	void							SetGraph(GraphNode* Graph);

private:
	void							OnGraphNeedsRepaint();
	void							Init();

	virtual void					paintGL();
	virtual void					mouseMoveEvent(QMouseEvent* event);
	virtual void					mousePressEvent(QMouseEvent* event);
	virtual void					mouseReleaseEvent(QMouseEvent* event);
	virtual void					keyPressEvent(QKeyEvent* event);

	void							Paint();

	void							OnMouseMove(QMouseEvent* event);
	void							OnMouseLeftDown(QMouseEvent* event);
	void							OnMouseLeftUp(QMouseEvent* event);
	void							OnMouseRightDown(QMouseEvent* event);
	void							OnMouseRightUp(QMouseEvent* event);
	void							OnKeyPress(QKeyEvent* event);

	/// Command-accessible functions
	NodeWidget*						AddNode(Node* Nd);

	/// All wigdets on the graph
	GraphNode*						Graph;

	/// Mapping from node to widget
	map<Node*, NodeWidget*>			WidgetMap;

	/// Will be called when a widget wants to repaint
	void							OnWidgetRepaint();

	/// Operators currectly selected
	set<NodeWidget*>				SelectedNodes;

	/// State machine
	enum class State {
		DEFAULT,
		MOVE_NODES,
		SELECT_RECTANGLE,
		CONNECT_TO_NODE,
		CONNECT_TO_SLOT,
	};

	State							CurrentState;

	/// True if mouse movement was made during STATE_MOVE_NODES
	bool							NodesMoved;

	/// True if connection is estimated to be valid
	bool							ConnectionValid;

	/// Mouse position when clicked
	Vec2							OriginalMousePos;
	Vec2							CurrentMousePos;

	/// Clicked widget for some operations
	NodeWidget*						ClickedWidget;
	int								ClickedSlot;

	/// Hovered widget and slot
	NodeWidget*						HoveredWidget;
	int								HoveredSlot;

	void							DeselectAll();
	void							StorePositionOfSelectedNodes();

	/// Finds which widget and slot is hovered by the mouse pointer
	bool							UpdateHoveredWidget(Vec2 MousePos);
};

