#include "grapheditor.h"
#include "nodewidget.h"
#include "../util/uipainter.h"
#include "../commands/graphcommands.h"
#include "../document.h"
//#include "operatorPrototypes.h"
#include <zengine.h>
#include <QBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>

GraphEditor::GraphEditor( QWidget* Parent )
	: QGLWidget(Parent)
	, Graph(NULL)
{
	//QPushButton* button = new QPushButton(this);
	//button->move(10, 10);
	//button->setText("nyirfa");

	setMouseTracking(true);

	CurrentState = State::DEFAULT;
	ClickedWidget = NULL;
	HoveredWidget = NULL;
	HoveredSlot = -1;

	EventZengineInitDone += MakeDelegate(this, &GraphEditor::Init);
}

void GraphEditor::paintGL()
{
	ThePainter->Set(width(), height());

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
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
	nw->EventRepaint += MakeDelegate(this, &GraphEditor::OnWidgetRepaint);
	WidgetMap[Nd] = nw;
	Graph->Widgets.push_back(nw);
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
	ThePainter->Color.SetValue(Vec4(1, 1, 1, 1));
	for (int i=Graph->Widgets.size()-1; i>=0; i--) {
		NodeWidget* ndWidget = Graph->Widgets[i];
		Node* node = ndWidget->GetNode();
		for (int i=0; i<node->Slots.size(); i++) {
			Slot* slot = node->Slots[i];
			Node* connectedOp = slot->GetConnectedNode();
			if (connectedOp) {
				NodeWidget* connectedOpWidget = GetNodeWidget(connectedOp);
				if (connectedOpWidget != NULL) {
					/// Draw connection
					Vec2 p1 = connectedOpWidget->GetOutputPosition();
					Vec2 p2 = ndWidget->GetInputPosition(i);
					ThePainter->DrawLine(p1.X, p1.Y, p2.X, p2.Y);
				}
			}
		}		
	}

	if (CurrentState == State::CONNECT_TO_NODE) {
		Vec2 from = ClickedWidget->GetInputPosition(ClickedSlot);
		Vec2 to = CurrentMousePos;
		ThePainter->Color.SetValue(Vec4(1, 1, 1, 0.7));
		ThePainter->DrawLine(from, to);
	}

	if (CurrentState == State::CONNECT_TO_SLOT) {
		Vec2 from = ClickedWidget->GetOutputPosition();
		Vec2 to = CurrentMousePos;
		ThePainter->Color.SetValue(Vec4(1, 1, 1, 0.7));
		ThePainter->DrawLine(from, to);
	}

	/// Draw nodes
	for (int i=Graph->Widgets.size()-1; i>=0; i--) {
		Graph->Widgets[i]->Paint(this);
	}

	/// Draw selection rectangle
	if (CurrentState == State::SELECT_RECTANGLE)
	{
		ThePainter->Color.SetValue(Vec4(0.4, 0.9, 1, 0.1));
		ThePainter->DrawBox(OriginalMousePos, CurrentMousePos - OriginalMousePos);
		ThePainter->Color.SetValue(Vec4(0.6, 0.9, 1, 0.6));
		ThePainter->DrawRect(OriginalMousePos, CurrentMousePos - OriginalMousePos);
	}
}

NodeWidget* GraphEditor::GetNodeWidget( Node* Op )
{
	auto it = WidgetMap.find(Op);
	return (it != WidgetMap.end()) ? it->second : NULL;
}

bool IsInsideRect(Vec2 Position, Vec2 Topleft, Vec2 Size) {
	return (Position.X >= Topleft.X && Position.X <= Topleft.X + Size.X 
		&& Position.Y >= Topleft.Y && Position.Y <= Topleft.Y + Size.Y);
}

bool HasIntersection(Vec2 Pos1, Vec2 Size1, Vec2 Pos2, Vec2 Size2)
{
	if (Size1.X < 0) {
		Pos1.X += Size1.X;
		Size1.X = -Size1.X;
	}
	if (Size1.Y < 0) {
		Pos1.Y += Size1.Y;
		Size1.Y = -Size1.Y;
	}
	if (Size2.X < 0) {
		Pos2.X += Size2.X;
		Size2.X = -Size2.X;
	}
	if (Size2.Y < 0) {
		Pos2.Y += Size2.Y;
		Size2.Y = -Size2.Y;
	}
	return !(	
		Pos1.X + Size1.X <= Pos2.X ||
		Pos1.Y + Size1.Y <= Pos2.Y ||
		Pos2.X + Size2.X <= Pos1.X ||
		Pos2.Y + Size2.Y <= Pos1.Y);
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
				}
				StorePositionOfSelectedNodes();
				NodesMoved = false;
				ClickedWidget = HoveredWidget;
				CurrentState = State::MOVE_NODES;

				/// Put node on top
				Graph->Widgets.erase(
					std::find(Graph->Widgets.begin(), Graph->Widgets.end(), HoveredWidget));
				Graph->Widgets.insert(Graph->Widgets.begin(), HoveredWidget);
			}
		} else {
			/// No widget was pressed, start rectangular selection
			CurrentState = State::SELECT_RECTANGLE;
			DeselectAll();
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
		foreach (NodeWidget* ow, Graph->Widgets)
		{
			if (ow->Selected) SelectedNodes.insert(ow);
		}
		CurrentState = State::DEFAULT;
		update();
		break;
	case State::CONNECT_TO_NODE:
		if (ConnectionValid) {
			Node* node = HoveredWidget->GetNode();
			Slot* slot = ClickedWidget->GetNode()->Slots[ClickedSlot];
			TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
		}
		update();
		CurrentState = State::DEFAULT;
		break;
	case State::CONNECT_TO_SLOT:
		if (ConnectionValid) {
			Node* node = ClickedWidget->GetNode();
			Slot* slot = HoveredWidget->GetNode()->Slots[HoveredSlot];
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
			Slot* slot = HoveredWidget->GetNode()->Slots[HoveredSlot];
			if (slot->GetConnectedNode()) {
				TheCommandStack->Execute(new ConnectNodeToSlotCommand(NULL, slot));
			}
			update();
		}
		return;
	}

	//Node* op = ThePrototypes->AskUser(event->globalPos());
	//if (op)
	//{
	//	TheCommandStack->Execute(new CreateOperatorCommand(op, this));
	//	OperatorWidget* ow = GetOperatorWidget(op);
	//	TheCommandStack->Execute(new MoveOperatorCommand(ow, Vec2(event->x(), event->y())));
	//}
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
		foreach (NodeWidget* ow, Graph->Widgets)
		{
			ow->Selected = HasIntersection(OriginalMousePos, 
				CurrentMousePos - OriginalMousePos, ow->Position, ow->Size);
		}
		update();
		break;
	case State::CONNECT_TO_NODE:
		ConnectionValid = false;
		UpdateHoveredWidget(mousePos);
		if (HoveredWidget && HoveredWidget != ClickedWidget) {
			if (HoveredWidget->GetNode()->GetType() 
				== ClickedWidget->GetNode()->Slots[ClickedSlot]->GetType()) {
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
				== HoveredWidget->GetNode()->Slots[HoveredSlot]->GetType()) {
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
	for (int i=0; i<Graph->Widgets.size(); i++)
	{
		NodeWidget* ow = Graph->Widgets[i];
		if (IsInsideRect(MousePos, ow->Position, ow->Size))
		{
			hovered = ow;
			for (int o=0; o<ow->Slots.size(); o++)
			{
				NodeWidget::SlotWidget* sw = ow->Slots[o];
				if (IsInsideRect(MousePos, ow->Position + sw->Position, sw->Size)) {
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
	default: break;
	}
}

void GraphEditor::SetGraph( NodeGraph* Graph )
{
	this->Graph = Graph;
}
