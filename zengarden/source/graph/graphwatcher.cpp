#include "graphwatcher.h"
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


GraphWatcher::GraphWatcher(GraphNode* graph, GLWatcherWidget* parent)
  : Watcher(graph, parent) 
{
  GetGLWidget()->setMouseTracking(true);
  GetGLWidget()->setFocusPolicy(Qt::ClickFocus);

  mCurrentState = State::DEFAULT;
  mClickedWidget = NULL;
  mHoveredWidget = NULL;
  mHoveredSlotIndex = -1;

  GetGLWidget()->OnPaint += Delegate(this, &GraphWatcher::Paint);
  GetGLWidget()->OnMousePress += Delegate(this, &GraphWatcher::HandleMousePress);
  GetGLWidget()->OnMouseRelease += Delegate(this, &GraphWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GraphWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GraphWatcher::HandleKeyPress);
}


void GraphWatcher::Paint(GLWidget*) {
  GLWidget* glWidget = GetGLWidget();

  TheDrawingAPI->OnContextSwitch();

  ThePainter->Set(glWidget->width(), glWidget->height());

  glClearColor(0.26f, 0.26f, 0.26f, 1.0f);
  TheDrawingAPI->Clear();

  /// Draw connections
  ThePainter->Color.Set(Vec4(1, 1, 1, 1));
  const vector<Node*>& nodes = GetGraph()->Widgets.GetMultiNodes();
  for (int i = nodes.size() - 1; i >= 0; i--) {
    NodeWidget* ndWidget = static_cast<NodeWidget*>(nodes[i]);
    Node* node = ndWidget->GetNode();
    for (int i = 0; i < ndWidget->mWidgetSlots.size(); i++) {
      Slot* slot = ndWidget->mWidgetSlots[i]->mSlot;
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

  if (mCurrentState == State::CONNECT_TO_NODE) {
    Vec2 from = mClickedWidget->GetInputPosition(mClickedSlotIndex);
    Vec2 to = mCurrentMousePos;
    ThePainter->Color.Set(Vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  if (mCurrentState == State::CONNECT_TO_SLOT) {
    Vec2 from = mClickedWidget->GetOutputPosition();
    Vec2 to = mCurrentMousePos;
    ThePainter->Color.Set(Vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  /// Draw nodes
  for (int i = nodes.size() - 1; i >= 0; i--) {
    static_cast<NodeWidget*>(GetGraph()->Widgets[i])->Paint(this);
  }

  /// Draw selection rectangle
  if (mCurrentState == State::SELECT_RECTANGLE) {
    ThePainter->Color.Set(Vec4(0.4, 0.9, 1, 0.1));
    ThePainter->DrawBox(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
    ThePainter->Color.Set(Vec4(0.6, 0.9, 1, 0.6));
    ThePainter->DrawRect(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
  }
}


void GraphWatcher::HandleGraphNeedsRepaint() {
  GetGLWidget()->update();
}


void GraphWatcher::HandleMousePress(GLWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}


void GraphWatcher::HandleMouseRelease(GLWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  } else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}


NodeWidget* GraphWatcher::AddNode(Node* node) {
  NodeWidget* widget = new NodeWidget(node);
  widget->OnRepaint += Delegate(this, &GraphWatcher::HandleWidgetRepaint);
  mWidgetMap[node] = widget;
  GetGraph()->Widgets.Connect(widget);
  GetGLWidget()->update();
  return widget;
}


void GraphWatcher::HandleWidgetRepaint() {
  GetGLWidget()->update();
}


NodeWidget* GraphWatcher::GetNodeWidget(Node* node) {
  auto it = mWidgetMap.find(node);
  return (it != mWidgetMap.end()) ? it->second : NULL;
}


bool IsInsideRect(Vec2 position, Vec2 topleft, Vec2 size) {
  return (position.x >= topleft.x && position.x <= topleft.x + size.x
          && position.y >= topleft.y && position.y <= topleft.y + size.y);
}


bool HasIntersection(Vec2 pos1, Vec2 size1, Vec2 pos2, Vec2 size2) {
  if (size1.x < 0) {
    pos1.x += size1.x;
    size1.x = -size1.x;
  }
  if (size1.y < 0) {
    pos1.y += size1.y;
    size1.y = -size1.y;
  }
  if (size2.x < 0) {
    pos2.x += size2.x;
    size2.x = -size2.x;
  }
  if (size2.y < 0) {
    pos2.y += size2.y;
    size2.y = -size2.y;
  }
  return !(
    pos1.x + size1.x <= pos2.x ||
    pos1.y + size1.y <= pos2.y ||
    pos2.x + size2.x <= pos1.x ||
    pos2.y + size2.y <= pos1.y);
}


void GraphWatcher::DeselectAll() {
  for (NodeWidget* ow : mSelectedNodes) {
    ow->mIsSelected = false;
  }
  mSelectedNodes.clear();
}


void GraphWatcher::StorePositionOfSelectedNodes() {
  for (NodeWidget* nodeWidget : mSelectedNodes) {
    nodeWidget->mOriginalPosition = nodeWidget->GetNode()->GetPosition();
    nodeWidget->mOriginalSize = nodeWidget->GetNode()->GetSize();
  }
}


void GraphWatcher::HandleMouseLeftDown(QMouseEvent* event) {
  Vec2 mousePos(event->x(), event->y());
  mOriginalMousePos = mousePos;
  mCurrentMousePos = mousePos;

  switch (mCurrentState) {
    case State::DEFAULT:
      if (mHoveredWidget) {
        if ((event->modifiers() & Qt::AltModifier) > 0) {
          if (mHoveredSlotIndex >= 0) {
            /// Start connecting from slot to node
            mCurrentState = State::CONNECT_TO_NODE;
            mClickedWidget = mHoveredWidget;
            mClickedSlotIndex = mHoveredSlotIndex;
            mIsConnectionValid = false;
            DeselectAll();
          } else {
            /// Start connecting from node to slot
            mCurrentState = State::CONNECT_TO_SLOT;
            mClickedWidget = mHoveredWidget;
            mClickedSlotIndex = -1;
            mIsConnectionValid = false;
            DeselectAll();
          }
        } else {
          if (!mHoveredWidget->mIsSelected) {
            /// Select node
            DeselectAll();
            mHoveredWidget->mIsSelected = true;
            mSelectedNodes.insert(mHoveredWidget);
            mWatcherWidget->onSelectNode(mHoveredWidget->GetNode());
          }
          StorePositionOfSelectedNodes();
          mAreNodesMoved = false;
          mClickedWidget = mHoveredWidget;
          mCurrentState = State::MOVE_NODES;

          /// Put node on top
          GetGraph()->Widgets.ChangeNodeIndex(mHoveredWidget, 0);
        }
      } else {
        /// No widget was pressed, start rectangular selection
        mCurrentState = State::SELECT_RECTANGLE;
        DeselectAll();
        mWatcherWidget->onSelectNode(nullptr);
      }
      break;
    case State::CONNECT_TO_NODE:

      break;
    default: break;
  }
  GetGLWidget()->update();
}


void GraphWatcher::HandleMouseLeftUp(QMouseEvent* event) {
  Vec2 mousePos(event->x(), event->y());
  switch (mCurrentState) {
    case State::MOVE_NODES:
      if (mAreNodesMoved) {
        for (NodeWidget* ow : mSelectedNodes) {
          Vec2 pos = ow->GetNode()->GetPosition();
          TheCommandStack->Execute(
            new MoveNodeCommand(ow->GetNode(), pos, ow->mOriginalPosition));
        }
      } else {
        DeselectAll();
        mClickedWidget->mIsSelected = true;
        mSelectedNodes.insert(mClickedWidget);
        GetGLWidget()->update();
      }
      mCurrentState = State::DEFAULT;
      break;
    case State::SELECT_RECTANGLE:
      for (Node* node : GetGraph()->Widgets.GetMultiNodes()) {
        NodeWidget* widget = static_cast<NodeWidget*>(node);
        if (widget->mIsSelected) mSelectedNodes.insert(widget);
      }
      mCurrentState = State::DEFAULT;
      GetGLWidget()->update();
      break;
    case State::CONNECT_TO_NODE:
      if (mIsConnectionValid) {
        Node* node = mHoveredWidget->GetNode();
        Slot* slot = mClickedWidget->GetNode()->mSlots[mClickedSlotIndex];
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
      }
      GetGLWidget()->update();
      mCurrentState = State::DEFAULT;
      break;
    case State::CONNECT_TO_SLOT:
      if (mIsConnectionValid) {
        Node* node = mClickedWidget->GetNode();
        Slot* slot = mHoveredWidget->GetNode()->mSlots[mHoveredSlotIndex];
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
      }
      GetGLWidget()->update();
      mCurrentState = State::DEFAULT;
      break;
    case State::DEFAULT:
      break;
  }
}


void GraphWatcher::HandleMouseRightDown(QMouseEvent* event) {
  if ((event->modifiers() & Qt::AltModifier) > 0) {
    if (mHoveredSlotIndex >= 0) {
      /// Remove connection
      Slot* slot = mHoveredWidget->GetNode()->mSlots[mHoveredSlotIndex];
      if (slot->GetNode()) {
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(NULL, slot));
      }
    }
    return;
  }

  Node* node = ThePrototypes->AskUser(mWatcherWidget, event->globalPos());
  if (node) {
    TheCommandStack->Execute(new CreateNodeCommand(node, this));
    TheCommandStack->Execute(new MoveNodeCommand(node, Vec2(event->x(), event->y())));
  }
}


void GraphWatcher::HandleMouseRightUp(QMouseEvent* event) {
}


void GraphWatcher::HandleMouseMove(GLWidget*, QMouseEvent* event) {
  Vec2 mousePos(event->x(), event->y());
  mCurrentMousePos = mousePos;
  switch (mCurrentState) {
    case State::MOVE_NODES:
    {
      mAreNodesMoved = true;
      Vec2 mouseDiff = mousePos - mOriginalMousePos;
      for (NodeWidget* ow : mSelectedNodes) {
        ow->GetNode()->SetPosition(ow->mOriginalPosition + mouseDiff);
      }
    }
    break;
    case State::SELECT_RECTANGLE:
      for (Node* w : GetGraph()->Widgets.GetMultiNodes()) {
        NodeWidget* widget = static_cast<NodeWidget*>(w);
        Node* node = widget->GetNode();
        widget->mIsSelected = HasIntersection(mOriginalMousePos,
            mCurrentMousePos - mOriginalMousePos, node->GetPosition(), node->GetSize());
      }
      GetGLWidget()->update();
      break;
    case State::CONNECT_TO_NODE:
      mIsConnectionValid = false;
      UpdateHoveredWidget(mousePos);
      if (mHoveredWidget && mHoveredWidget != mClickedWidget) {
        if (mClickedWidget->GetNode()->mSlots[mClickedSlotIndex]->DoesAcceptType(
          mHoveredWidget->GetNode()->GetType())) {
          mIsConnectionValid = true;
        }
      }
      GetGLWidget()->update();
      break;
    case State::CONNECT_TO_SLOT:
      mIsConnectionValid = false;
      UpdateHoveredWidget(mousePos);
      if (mHoveredSlotIndex >= 0 && mHoveredWidget != mClickedWidget) {
        if (mHoveredWidget->GetNode()->mSlots[mHoveredSlotIndex]->DoesAcceptType(
          mClickedWidget->GetNode()->GetType())) {
          mIsConnectionValid = true;
        }
      }
      GetGLWidget()->update();
      break;
    default:
      if (UpdateHoveredWidget(mousePos)) GetGLWidget()->update();
      break;
  }
}


bool GraphWatcher::UpdateHoveredWidget(Vec2 mousePos) {
  NodeWidget* hovered = nullptr;
  int slot = -1;
  for (Node* w : GetGraph()->Widgets.GetMultiNodes()) {
    NodeWidget* widget = static_cast<NodeWidget*>(w);
    Node* node = widget->GetNode();
    if (IsInsideRect(mousePos, node->GetPosition(), node->GetSize())) {
      hovered = widget;
      for (int o = 0; o < widget->mWidgetSlots.size(); o++) {
        NodeWidget::WidgetSlot* sw = widget->mWidgetSlots[o];
        if (IsInsideRect(mousePos, node->GetPosition() + sw->mPosition, sw->mSize)) {
          slot = o;
          break;
        }
      }
      break;
    }
  }

  bool changed =
    hovered != mHoveredWidget || (hovered != nullptr && slot != mHoveredSlotIndex);
  mHoveredWidget = hovered;
  mHoveredSlotIndex = slot;
  return changed;
}


void GraphWatcher::HandleKeyPress(GLWidget*, QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Delete:
      //new DeleteOperatorCommand()
      //TheCommandStack->Execute(new CreateOperatorCommand(op, this));
      break;

      /// Space opens watcher
    case Qt::Key_Space:
      if (mSelectedNodes.size() == 1) {
        mWatcherWidget->onWatchNode((*mSelectedNodes.begin())->GetNode(), mWatcherWidget);
      }
      break;

    default: break;
  }
}


GraphNode* GraphWatcher::GetGraph() {
  return static_cast<GraphNode*>(GetNode());
}
