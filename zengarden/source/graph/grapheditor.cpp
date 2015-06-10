#include "grapheditor.h"
#include "nodewidget.h"
#include "../util/uipainter.h"
#include "../commands/graphcommands.h"
#include "../document.h"
#include "prototypes.h"
#include <zengine.h>
#include <QBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

GraphEditor::GraphEditor(WatcherWidget* Parent, QGLWidget* Share)
	: QGLWidget(Parent, Share)
	, Graph(NULL)
	, ParentWidget(Parent)
{
	//QPushButton* button = new QPushButton(this);
	//button->move(10, 10);
	//button->setText("nyirfa");

	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);

	CurrentState = State::DEFAULT;
	ClickedWidget = NULL;
	HoveredWidget = NULL;
	HoveredSlot = -1;

	OnZengineInitDone += Delegate(this, &GraphEditor::Init);
}

void GraphEditor::paintGL()
{
	TheDrawingAPI->OnContextSwitch();
	ThePainter->Set(width(), height());

	glClearColor(0.26f, 0.26f, 0.26f, 1.0f);
	TheDrawingAPI->Clear();
	
	Paint();	
}

void GraphEditor::Init()
{}

void GraphEditor::mouseMoveEvent( QMouseEvent* event )
{
	OnMouseMove(event);
}

void GraphEditor::OnGraphNeedsRepaint()
{
	update();
}

void GraphEditor::mousePressEvent( QMouseEvent* event )
{
	if (event->button() == Qt::LeftButton)
	{
		OnMouseLeftDown(event);
	} 
	else if (event->button() == Qt::RightButton) 
	{
		OnMouseRightDown(event);
	}
}

void GraphEditor::mouseReleaseEvent( QMouseEvent* event )
{
	if (event->button() == Qt::LeftButton)
	{
		OnMouseLeftUp(event);
	}
	else if (event->button() == Qt::RightButton) 
	{
		OnMouseRightUp(event);
	}
}

void GraphEditor::keyPressEvent( QKeyEvent* event )
{
	OnKeyPress(event);
}

NodeWidget* GraphEditor::AddNode( Node* Nd )
{
	NodeWidget* nw = new NodeWidget(Nd);
	nw->EventRepaint += Delegate(this, &GraphEditor::OnWidgetRepaint);
	WidgetMap[Nd] = nw;
	Graph->Widgets.Connect(nw);
	update();
	return nw;
}

void GraphEditor::OnWidgetRepaint()
{
	update();
}

void GraphEditor::Paint()
{
	/// Draw connections
	ThePainter->Color.Set(Vec4(1, 1, 1, 1));
	const vector<Node*>& nodes = Graph->Widgets.GetMultiNodes();
	for (int i=nodes.size()-1; i>=0; i--) {
		NodeWidget* ndWidget = static_cast<NodeWidget*>(nodes[i]);
		Node* node = ndWidget->GetNode();
		for (int i=0; i<ndWidget->WidgetSlots.size(); i++) {
			Slot* slot = ndWidget->WidgetSlots[i]->TheSlot;
			Node* connectedOp = slot->GetNode();
			if (connectedOp) {
				NodeWidget* connectedOpWidget = GetNodeWidget(connectedOp);
				if (connectedOpWidget != NULL) {
					/// Draw connection
					Vec2 p1 = connectedOpWidget->GetOutputPosition();
					Vec2 p2 = ndWidget->GetInputPosition(i);
					ThePainter->DrawLine(p1.x, p1.y, p2.x, p2.y);
				}
			}
		}		
	}

	if (CurrentState == State::CONNECT_TO_NODE) {
		Vec2 from = ClickedWidget->GetInputPosition(ClickedSlot);
		Vec2 to = CurrentMousePos;
		ThePainter->Color.Set(Vec4(1, 1, 1, 0.7));
		ThePainter->DrawLine(from, to);
	}

	if (CurrentState == State::CONNECT_TO_SLOT) {
		Vec2 from = ClickedWidget->GetOutputPosition();
		Vec2 to = CurrentMousePos;
		ThePainter->Color.Set(Vec4(1, 1, 1, 0.7));
		ThePainter->DrawLine(from, to);
	}

	/// Draw nodes
	for (int i=nodes.size()-1; i>=0; i--) {
		static_cast<NodeWidget*>(Graph->Widgets[i])->Paint(this);
	}

	/// Draw selection rectangle
	if (CurrentState == State::SELECT_RECTANGLE)
	{
		ThePainter->Color.Set(Vec4(0.4, 0.9, 1, 0.1));
		ThePainter->DrawBox(OriginalMousePos, CurrentMousePos - OriginalMousePos);
		ThePainter->Color.Set(Vec4(0.6, 0.9, 1, 0.6));
		ThePainter->DrawRect(OriginalMousePos, CurrentMousePos - OriginalMousePos);
	}
}

NodeWidget* GraphEditor::GetNodeWidget( Node* Op )
{
	auto it = WidgetMap.find(Op);
	return (it != WidgetMap.end()) ? it->second : NULL;
}

bool IsInsideRect(Vec2 Position, Vec2 Topleft, Vec2 Size) {
	return (Position.x >= Topleft.x && Position.x <= Topleft.x + Size.x 
		&& Position.y >= Topleft.y && Position.y <= Topleft.y + Size.y);
}

bool HasIntersection(Vec2 Pos1, Vec2 Size1, Vec2 Pos2, Vec2 Size2)
{
	if (Size1.x < 0) {
		Pos1.x += Size1.x;
		Size1.x = -Size1.x;
	}
	if (Size1.y < 0) {
		Pos1.y += Size1.y;
		Size1.y = -Size1.y;
	}
	if (Size2.x < 0) {
		Pos2.x += Size2.x;
		Size2.x = -Size2.x;
	}
	if (Size2.y < 0) {
		Pos2.y += Size2.y;
		Size2.y = -Size2.y;
	}
	return !(	
		Pos1.x + Size1.x <= Pos2.x ||
		Pos1.y + Size1.y <= Pos2.y ||
		Pos2.x + Size2.x <= Pos1.x ||
		Pos2.y + Size2.y <= Pos1.y);
}

void GraphEditor::DeselectAll()
{
	foreach (NodeWidget* ow, SelectedNodes)
	{
		ow->Selected = false;
	}
	SelectedNodes.clear();
}

void GraphEditor::StorePositionOfSelectedNodes()
{
	foreach (NodeWidget* ow, SelectedNodes)
	{
		ow->OriginalPosition = ow->Position;
		ow->OriginalSize = ow->Size;
	}
}

void GraphEditor::OnMouseLeftDown( QMouseEvent* event )
{
	Vec2 mousePos(event->x(), event->y());
	OriginalMousePos = mousePos;
	CurrentMousePos = mousePos;

	switch (CurrentState)
	{
	case State::DEFAULT:
		if (HoveredWidget) 
		{
			if ((event->modifiers() & Qt::AltModifier) > 0)
			{
				if (HoveredSlot >= 0) {
					/// Start connecting from slot to node
					CurrentState = State::CONNECT_TO_NODE;
					ClickedWidget = HoveredWidget;
					ClickedSlot = HoveredSlot;
					ConnectionValid = false;
					DeselectAll();
				} else {
					/// Start connecting from node to slot
					CurrentState = State::CONNECT_TO_SLOT;
					ClickedWidget = HoveredWidget;
					ClickedSlot = -1;
					ConnectionValid = false;
					DeselectAll();
				}
			} else {
				if (!HoveredWidget->Selected)
				{
					/// Select node
					DeselectAll();
					HoveredWidget->Selected = true;
					SelectedNodes.insert(HoveredWidget);
					ParentWidget->OnSelectNode(HoveredWidget->GetNode());
				}
				StorePositionOfSelectedNodes();
				NodesMoved = false;
				ClickedWidget = HoveredWidget;
				CurrentState = State::MOVE_NODES;

				/// Put node on top
				Graph->Widgets.ChangeNodeIndex(HoveredWidget, 0);
			}
		} else {
			/// No widget was pressed, start rectangular selection
			CurrentState = State::SELECT_RECTANGLE;
			DeselectAll();
			ParentWidget->OnSelectNode(nullptr);
		}
		break;
	case State::CONNECT_TO_NODE:

		break;
	default: break;
	}
	update();
}


void GraphEditor::OnMouseLeftUp( QMouseEvent* event )
{
	Vec2 mousePos(event->x(), event->y());
	switch (CurrentState)
	{
	case State::MOVE_NODES:
		if (NodesMoved)
		{
			foreach (NodeWidget* ow, SelectedNodes)
			{
				Vec2 pos = ow->Position;
				ow->Position = ow->OriginalPosition;
				TheCommandStack->Execute(new MoveNodeCommand(ow, pos));
			}
		} else {
			DeselectAll();
			ClickedWidget->Selected = true;
			SelectedNodes.insert(ClickedWidget);
			update();
		}
		CurrentState = State::DEFAULT;
		break;
	case State::SELECT_RECTANGLE:
		for (Node* node : Graph->Widgets.GetMultiNodes())
		{
			NodeWidget* widget = static_cast<NodeWidget*>(node);
			if (widget->Selected) SelectedNodes.insert(widget);
		}
		CurrentState = State::DEFAULT;
		update();
		break;
	case State::CONNECT_TO_NODE:
		if (ConnectionValid) {
			Node* node = HoveredWidget->GetNode();
			Slot* slot = ClickedWidget->GetNode()->mSlots[ClickedSlot];
			TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
		}
		update();
		CurrentState = State::DEFAULT;
		break;
	case State::CONNECT_TO_SLOT:
		if (ConnectionValid) {
			Node* node = ClickedWidget->GetNode();
			Slot* slot = HoveredWidget->GetNode()->mSlots[HoveredSlot];
			TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
		}
		update();
		CurrentState = State::DEFAULT;
		break;
	case State::DEFAULT:
		break;
	}
}

void GraphEditor::OnMouseRightDown( QMouseEvent* event )
{
	if ((event->modifiers() & Qt::AltModifier) > 0) {
		if (HoveredSlot >= 0) {
			/// Remove connection
			Slot* slot = HoveredWidget->GetNode()->mSlots[HoveredSlot];
			if (slot->GetNode()) {
				TheCommandStack->Execute(new ConnectNodeToSlotCommand(NULL, slot));
			}
			update();
		}
		return;
	}

	Node* node = ThePrototypes->AskUser(this, event->globalPos());
	if (node)
	{
		TheCommandStack->Execute(new CreateNodeCommand(node, this));
		NodeWidget* nodeWidget = GetNodeWidget(node);
		TheCommandStack->Execute(new MoveNodeCommand(nodeWidget, Vec2(event->x(), event->y())));
	}
}

void GraphEditor::OnMouseRightUp( QMouseEvent* event )
{

}

void GraphEditor::OnMouseMove( QMouseEvent* event )
{
	Vec2 mousePos(event->x(), event->y());
	CurrentMousePos = mousePos;
	switch (CurrentState)
	{
	case State::MOVE_NODES:
		{
			NodesMoved = true;
			Vec2 mouseDiff = mousePos - OriginalMousePos;
			foreach (NodeWidget* ow, SelectedNodes)
			{
				ow->Position = ow->OriginalPosition + mouseDiff;
			}
			update();
		}
		break;
	case State::SELECT_RECTANGLE:
		for (Node* node : Graph->Widgets.GetMultiNodes())
		{
			NodeWidget* widget = static_cast<NodeWidget*>(node);
			widget->Selected = HasIntersection(OriginalMousePos,
				CurrentMousePos - OriginalMousePos, widget->Position, widget->Size);
		}
		update();
		break;
	case State::CONNECT_TO_NODE:
		ConnectionValid = false;
		UpdateHoveredWidget(mousePos);
		if (HoveredWidget && HoveredWidget != ClickedWidget) {
			if (HoveredWidget->GetNode()->GetType() 
				== ClickedWidget->GetNode()->mSlots[ClickedSlot]->GetType()) {
					ConnectionValid = true;
			}
		}
		update();
		break;
	case State::CONNECT_TO_SLOT:
		ConnectionValid = false;
		UpdateHoveredWidget(mousePos); 
		if (HoveredSlot >= 0 && HoveredWidget != ClickedWidget) {
			if (ClickedWidget->GetNode()->GetType() 
				== HoveredWidget->GetNode()->mSlots[HoveredSlot]->GetType()) {
					ConnectionValid = true;
			}
		}
		update();
		break;
	default:
		if (UpdateHoveredWidget(mousePos)) update();
		break;
	}
}

bool GraphEditor::UpdateHoveredWidget(Vec2 MousePos)
{
	NodeWidget* hovered = NULL;
	int slot = -1;
	for (Node* node : Graph->Widgets.GetMultiNodes())
	{
		NodeWidget* widget = static_cast<NodeWidget*>(node);
		if (IsInsideRect(MousePos, widget->Position, widget->Size))
		{
			hovered = widget;
			for (int o=0; o<widget->WidgetSlots.size(); o++)
			{
				NodeWidget::WidgetSlot* sw = widget->WidgetSlots[o];
				if (IsInsideRect(MousePos, widget->Position + sw->Position, sw->Size)) {
					slot = o;
					break;
				}
			}
			break;
		}
	}

	bool changed = hovered != HoveredWidget || (hovered != NULL && slot != HoveredSlot);
	HoveredWidget = hovered;
	HoveredSlot = slot;
	return changed;
}

void GraphEditor::OnKeyPress( QKeyEvent* event )
{
	switch (event->key())
	{
	case Qt::Key_Delete:
		//new DeleteOperatorCommand()
		//TheCommandStack->Execute(new CreateOperatorCommand(op, this));
		break;

	/// Space opens watcher
	case Qt::Key_Space:
		if (SelectedNodes.size() == 1) {
			ParentWidget->OnWatchNode((*SelectedNodes.begin())->GetNode(), ParentWidget);
		}
		break;

	default: break;
	}
}

void GraphEditor::SetGraph( GraphNode* Graph )
{
	this->Graph = Graph;
}
