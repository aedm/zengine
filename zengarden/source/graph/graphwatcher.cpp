#include "graphwatcher.h"
#include "nodewidget.h"
#include "../util/uipainter.h"
#include "../util/util.h"
#include "../commands/graphcommands.h"
#include "prototypes.h"
#include <zengine.h>
#include <QBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QPainter>
#include <QTimer>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>

GraphWatcher::GraphWatcher(Graph* graph, GLWatcherWidget* parent)
  : Watcher(graph, parent) 
{
  GetGLWidget()->setMouseTracking(true);
  GetGLWidget()->setFocusPolicy(Qt::ClickFocus);

  parent->setAcceptDrops(true);

  mCurrentState = State::DEFAULT;
  mClickedWidget = NULL;
  mHoveredWidget = NULL;
  mHoveredSlotIndex = -1;

  parent->mShareWidget->makeCurrent();
  for (Node* node : graph->mNodes.GetMultiNodes()) {
    NodeWidget* widget = new NodeWidget(node, this);
    mWidgetMap[node] = widget;
  }

  GetGLWidget()->OnPaint += Delegate(this, &GraphWatcher::Paint);
  GetGLWidget()->OnMousePress += Delegate(this, &GraphWatcher::HandleMousePress);
  GetGLWidget()->OnMouseRelease += Delegate(this, &GraphWatcher::HandleMouseRelease);
  GetGLWidget()->OnMouseMove += Delegate(this, &GraphWatcher::HandleMouseMove);
  GetGLWidget()->OnKeyPress += Delegate(this, &GraphWatcher::HandleKeyPress);
  GetGLWidget()->OnMouseWheel += Delegate(this, &GraphWatcher::HandleMouseWheel);
}


void GraphWatcher::Paint(GLWidget* glWidget) {
  TheDrawingAPI->OnContextSwitch();

  Vec2 canvasSize, topLeft;
  GetCanvasDimensions(canvasSize, topLeft);

  ThePainter->SetupViewport(glWidget->width(), glWidget->height(), topLeft, canvasSize);

  glClearColor(0.26f, 0.26f, 0.26f, 1.0f);
  TheDrawingAPI->Clear();

  /// Draw connections
  ThePainter->mColor.Set(Vec4(1, 1, 1, 1));
  for (Node* node : GetGraph()->mNodes.GetMultiNodes()) {
    NodeWidget* nodeWidget = mWidgetMap.at(node);
    for (int i = 0; i < nodeWidget->mWidgetSlots.size(); i++) {
      Slot* slot = nodeWidget->mWidgetSlots[i]->mSlot;
      if (slot->mIsMultiSlot) {
        for (Node* connectedNode : slot->GetMultiNodes()) {
          NodeWidget* connectedNodeWidget = GetNodeWidget(connectedNode);
          if (connectedNodeWidget != NULL) {
            /// Draw connection
            Vec2 p1 = connectedNodeWidget->GetOutputPosition();
            Vec2 p2 = nodeWidget->GetInputPosition(i);
            ThePainter->DrawLine(p1.x, p1.y, p2.x, p2.y);
          }
        }
      } else {
        /// TODO: remove code duplication
        Node* connectedNode = slot->GetAbstractNode();
        if (connectedNode) {
          NodeWidget* connectedNodeWidget = GetNodeWidget(connectedNode);
          if (connectedNodeWidget != NULL) {
            /// Draw connection
            Vec2 p1 = connectedNodeWidget->GetOutputPosition();
            Vec2 p2 = nodeWidget->GetInputPosition(i);
            ThePainter->DrawLine(p1.x, p1.y, p2.x, p2.y);
          }
        }
      }
    }
  }

  if (mCurrentState == State::CONNECT_TO_NODE) {
    Vec2 from = mClickedWidget->GetInputPosition(mClickedSlotIndex);
    Vec2 to = mCurrentMousePos;
    ThePainter->mColor.Set(Vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  if (mCurrentState == State::CONNECT_TO_SLOT) {
    Vec2 from = mClickedWidget->GetOutputPosition();
    Vec2 to = mCurrentMousePos;
    ThePainter->mColor.Set(Vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  /// Draw nodes
  for (Node* node : GetGraph()->mNodes.GetMultiNodes()) {
    mWidgetMap.at(node)->Paint();
  }

  /// Draw selection rectangle
  if (mCurrentState == State::SELECT_RECTANGLE) {
    ThePainter->mColor.Set(Vec4(0.4, 0.9, 1, 0.1));
    ThePainter->DrawBox(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
    ThePainter->mColor.Set(Vec4(0.6, 0.9, 1, 0.6));
    ThePainter->DrawRect(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
  }
}


void GraphWatcher::Update() {
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
  for (NodeWidget* widget : mSelectedNodeWidgets) {
    widget->mIsSelected = false;
  }
  mSelectedNodeWidgets.clear();
}


void GraphWatcher::StorePositionOfSelectedNodes() {
  for (NodeWidget* nodeWidget : mSelectedNodeWidgets) {
    nodeWidget->mOriginalPosition = nodeWidget->GetNode()->GetPosition();
    nodeWidget->mOriginalSize = nodeWidget->GetNode()->GetSize();
  }
}


void GraphWatcher::HandleMouseLeftDown(QMouseEvent* event) {
  Vec2 mousePos = MouseToWorld(event);
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
            mSelectedNodeWidgets.insert(mHoveredWidget);
            mWatcherWidget->onSelectNode(mHoveredWidget->GetNode());
          }
          StorePositionOfSelectedNodes();
          mAreNodesMoved = false;
          mClickedWidget = mHoveredWidget;
          mCurrentState = State::MOVE_NODES;

          /// Put node on top
          GetGraph()->mNodes.ChangeNodeIndex(mHoveredWidget->GetNode(), 0);
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
  Vec2 mousePos = MouseToWorld(event);
  switch (mCurrentState) {
    case State::MOVE_NODES:
      if (mAreNodesMoved) {
        for (NodeWidget* widget : mSelectedNodeWidgets) {
          Vec2 pos = widget->GetNode()->GetPosition();
          TheCommandStack->Execute(
            new MoveNodeCommand(widget->GetNode(), pos, widget->mOriginalPosition));
        }
      } else {
        DeselectAll();
        mClickedWidget->mIsSelected = true;
        mSelectedNodeWidgets.insert(mClickedWidget);
        GetGLWidget()->update();
      }
      mCurrentState = State::DEFAULT;
      break;
    case State::SELECT_RECTANGLE:
      for (Node* node : GetGraph()->mNodes.GetMultiNodes()) {
        NodeWidget* widget = mWidgetMap.at(node);
        if (widget->mIsSelected) mSelectedNodeWidgets.insert(widget);
      }
      mCurrentState = State::DEFAULT;
      GetGLWidget()->update();
      break;
    case State::CONNECT_TO_NODE:
      if (mIsConnectionValid) {
        Node* node = mHoveredWidget->GetNode();
        Slot* slot = mClickedWidget->mWidgetSlots[mClickedSlotIndex]->mSlot;
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
      }
      GetGLWidget()->update();
      mCurrentState = State::DEFAULT;
      break;
    case State::CONNECT_TO_SLOT:
      if (mIsConnectionValid) {
        Node* node = mClickedWidget->GetNode();
        Slot* slot = mHoveredWidget->mWidgetSlots[mHoveredSlotIndex]->mSlot;
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
      Slot* slot = mHoveredWidget->mWidgetSlots[mHoveredSlotIndex]->mSlot;
      if (slot->GetAbstractNode()) {
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(NULL, slot));
      }
    }
    return;
  }

  /// Pan canvas
  Vec2 mousePos = MouseToWorld(event);
  if (mCurrentState == State::DEFAULT) {
    mOriginalMousePos = Vec2(event->x(), event->y());
    mOriginalCenter = mCenter;
    mCurrentState = State::PAN_CANVAS;
  }
}


void GraphWatcher::HandleMouseRightUp(QMouseEvent* event) {
  if (mCurrentState == State::PAN_CANVAS) {
    mCurrentState = State::DEFAULT;
  }
}


void GraphWatcher::HandleMouseMove(GLWidget*, QMouseEvent* event) {
  Vec2 mousePos = MouseToWorld(event);
  mCurrentMousePos = mousePos;
  switch (mCurrentState) {
    case State::MOVE_NODES:
    {
      mAreNodesMoved = true;
      Vec2 mouseDiff = mousePos - mOriginalMousePos;
      for (NodeWidget* widget : mSelectedNodeWidgets) {
        widget->GetNode()->SetPosition(widget->mOriginalPosition + mouseDiff);
      }
    }
    break;
    case State::SELECT_RECTANGLE:
      for (Node* node : GetGraph()->mNodes.GetMultiNodes()) {
        NodeWidget* widget = mWidgetMap.at(node);
        widget->mIsSelected = HasIntersection(mOriginalMousePos,
            mCurrentMousePos - mOriginalMousePos, node->GetPosition(), node->GetSize());
      }
      GetGLWidget()->update();
      break;
    case State::CONNECT_TO_NODE:
      mIsConnectionValid = false;
      UpdateHoveredWidget(mousePos);
      if (mHoveredWidget && mHoveredWidget != mClickedWidget) {
        if (mClickedWidget->mWidgetSlots[mClickedSlotIndex]->mSlot->DoesAcceptType(
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
        if (mHoveredWidget->mWidgetSlots[mHoveredSlotIndex]->mSlot->DoesAcceptType(
          mClickedWidget->GetNode()->GetType())) {
          mIsConnectionValid = true;
        }
      }
      GetGLWidget()->update();
      break;
    case State::PAN_CANVAS:
      {
        Vec2 mousePixelPos(event->x(), event->y());
        Vec2 diff = mousePixelPos - mOriginalMousePos;
        mCenter = mOriginalCenter - diff * mZoomFactor;
        GetGLWidget()->update();
      }
      break;
    default:
      if (UpdateHoveredWidget(mousePos)) GetGLWidget()->update();
      break;
  }
}


void GraphWatcher::HandleMouseWheel(GLWidget*, QWheelEvent* event) {
  mZoomExponent -= event->delta();
  if (mZoomExponent < 0) mZoomExponent = 0;
  mZoomFactor = powf(2.0, float(mZoomExponent) / (120.0f * 4.0f));
  GetGLWidget()->update();
}


bool GraphWatcher::UpdateHoveredWidget(Vec2 mousePos) {
  NodeWidget* hovered = nullptr;
  int slot = -1;
  for (Node* node : GetGraph()->mNodes.GetMultiNodes()) {
    NodeWidget* widget = mWidgetMap.at(node);
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
      if (mSelectedNodeWidgets.size() > 0) {
        set<Node*>* selectedNodes = new set<Node*>();
        for (NodeWidget* nodeWidget : mSelectedNodeWidgets) {
          selectedNodes->insert(nodeWidget->GetNode());
        }
        TheCommandStack->Execute(new DeleteNodeCommand(selectedNodes));
        mWatcherWidget->onSelectNode(nullptr);
      }
      break;

      /// 1 opens watcher
    case Qt::Key_1:
      if (mSelectedNodeWidgets.size() == 1) {
        mWatcherWidget->onWatchNode(
          (*mSelectedNodeWidgets.begin())->GetNode(), mWatcherWidget);
      }
      break;

    case Qt::Key_Space:
    {
      Node* node = ThePrototypes->AskUser(mWatcherWidget, QCursor::pos());
      if (node) {
        TheCommandStack->Execute(new CreateNodeCommand(node, GetGraph()));
        TheCommandStack->Execute(new MoveNodeCommand(node, mCurrentMousePos));
      }
      break;
    }
    default: break;
  }
}


Graph* GraphWatcher::GetGraph() {
  return static_cast<Graph*>(GetNode());
}


void GraphWatcher::HandleSniffedMessage(NodeMessage message, Slot* slot,
                                        void* payload) {
  switch (message) {
    case NodeMessage::MULTISLOT_CONNECTION_ADDED: {
        Node* node = static_cast<Node*>(payload);
        NodeWidget* widget = new NodeWidget(node, this);
        mWidgetMap[node] = widget;
        GetGLWidget()->update();
      }
      break;
    case NodeMessage::MULTISLOT_CONNECTION_REMOVED: {
        Node* node = static_cast<Node*>(payload);
        auto it = mWidgetMap.find(node);
        DeselectAll();
        if (it != mWidgetMap.end()) {
          delete it->second;
          mWidgetMap.erase(it);
        }
        UpdateHoveredWidget(mCurrentMousePos);
        Update();
    }
      break;
    case NodeMessage::MULTISLOT_CLEARED:
      break;
    case NodeMessage::NEEDS_REDRAW:
      break;
    case NodeMessage::NODE_NAME_CHANGED:
      break;
    case NodeMessage::NODE_POSITION_CHANGED:
      break;
    default:
      break;
  }
}


Vec2 GraphWatcher::CanvasToWorld(const Vec2& canvasCoord) {
  Vec2 canvasSize, topLeft;
  GetCanvasDimensions(canvasSize, topLeft);
  return topLeft + canvasCoord * mZoomFactor;
}


Vec2 GraphWatcher::MouseToWorld(QMouseEvent* event) {
  Vec2 mouseCoord(event->x(), event->y());
  return CanvasToWorld(mouseCoord);
}

void GraphWatcher::GetCanvasDimensions(Vec2& oCanvasSize, Vec2& oTopLeft) {
  oCanvasSize = Vec2(GetGLWidget()->width(), GetGLWidget()->height()) * mZoomFactor;
  oTopLeft = mCenter - oCanvasSize / 2.0f;

  /// Consistent shape of nodes
  oTopLeft.x = floorf(oTopLeft.x);
  oTopLeft.y = floorf(oTopLeft.y);
}


void GraphWatcher::HandleDragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* mimeData = event->mimeData();
  if (!mimeData->hasUrls()) return;
  QList<QUrl> urlList = mimeData->urls();
  if (urlList.size() != 1) return;

  QString fileName = urlList.at(0).toLocalFile();
  if (fileName.endsWith(".obj")) event->acceptProposedAction();
}

/// TODO: remove code duplication with HandleDragEnterEvent
void GraphWatcher::HandleDropEvent(QDropEvent* event) {
  const QMimeData* mimeData = event->mimeData();
  if (!mimeData->hasUrls()) return;
  QList<QUrl> urlList = mimeData->urls();
  if (urlList.size() != 1) return;

  QString fileName = urlList.at(0).toLocalFile();
  QFileInfo fileInfo(fileName);
  if (fileInfo.suffix() == "obj") {
    Mesh* mesh = Util::LoadMesh(fileName);
    MeshNode* node = StaticMeshNode::Create(mesh);
    node->SetName(fileInfo.fileName().toStdString());
    TheCommandStack->Execute(new CreateNodeCommand(node, GetGraph()));

    Vec2 pos = CanvasToWorld(Vec2(event->pos().x(), event->pos().y()));
    TheCommandStack->Execute(new MoveNodeCommand(node, pos));
    event->acceptProposedAction();
  }
}
