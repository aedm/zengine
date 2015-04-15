#pragma once

#include <zengine.h>
#include <QFrame>
#include <QGLWidget>
#include <vector>

using namespace std;
class OperatorWidget;
class OperatorGraph;

class OperatorPanel: public QGLWidget { Q_OBJECT
	friend class CreateOperatorCommand;
	friend class OperatorWidget;

public:
	OperatorPanel(QWidget* Parent);	

	OperatorWidget*						GetOperatorWidget(Node* Nd);
	void								SetGraph(OperatorGraph* Graph);

private:
	void								OnGraphNeedsRepaint();
	void								Init();

	virtual void						paintGL();
	virtual void						mouseMoveEvent(QMouseEvent* event);
	virtual void						mousePressEvent(QMouseEvent* event);
	virtual void						mouseReleaseEvent(QMouseEvent* event);
	virtual void						keyPressEvent (QKeyEvent* event);

	void								Paint();

	void								OnMouseMove(QMouseEvent* event);
	void								OnMouseLeftDown(QMouseEvent* event);
	void								OnMouseLeftUp(QMouseEvent* event);
	void								OnMouseRightDown(QMouseEvent* event);
	void								OnMouseRightUp(QMouseEvent* event);
	void								OnKeyPress(QKeyEvent* event);

	/// Command-accessible functions
	OperatorWidget*						AddOperator(Node* Nd);

	/// All wigdets on the graph
	OperatorGraph*						Graph;

	/// Mapping from operator to widget
	map<Node*, OperatorWidget*>			WidgetMap;

	/// Will be called when a widget wants to repaint
	void								OnWidgetRepaint();

	/// Operators currectly selected
	set<OperatorWidget*>				SelectedOperators;

	/// State machine
	enum EState {
		STATE_VOID,
		STATE_MOVEOPERATORS,
		STATE_SELECTRECTANGLE,
		STATE_CONNECTTOOPERATOR,
		STATE_CONNECTTOSLOT,
	};

	EState								State;

	/// True if mouse movement was made during STATE_MOVEOPERATORS
	bool								OperatorsMoved;

	/// True if connection is estimated to be valid
	bool								ConnectionValid;

	/// Mouse position when clicked
	Vec2								OriginalMousePos;
	Vec2								CurrentMousePos;

	/// Clicked widget for some operations
	OperatorWidget*						ClickedWidget;
	int									ClickedSlot;

	/// Hovered widget and slot
	OperatorWidget*						HoveredWidget;
	int									HoveredSlot;

	void								DeselectAll();
	void								StorePositionOfSelectedOperators();

	/// Finds which widget and slot is hovered by the mouse pointer
	bool								UpdateHoveredWidget(Vec2 MousePos);
};

