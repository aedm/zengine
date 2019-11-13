#include "graphwatcher.h"
#include "nodewidget.h"
#include "../util/uipainter.h"
#include "../util/util.h"
#include "../commands/graphCommands.h"
#include "prototypes.h"
#include "../zengarden.h"
#include <zengine.h>
#include <QTimer>
#include <QMouseEvent>
#include <QMimeData>
#include <QFileInfo>

GraphWatcher::GraphWatcher(const std::shared_ptr<Node>& graph)
  : WatcherUi(graph) {
  mCurrentState = State::DEFAULT;
  mClickedWidget = nullptr;
  mHoveredWidget = nullptr;
  mHoveredSlotIndex = -1;
}


GraphWatcher::~GraphWatcher() {
  for (auto& it : mWidgetMap) {
    std::shared_ptr<NodeWidget> watcher = it.second;
    if (watcher->GetDirectNode()) watcher->GetDirectNode()->RemoveWatcher(watcher);
  }
}


void GraphWatcher::Paint(EventForwarderGlWidget* glWidget) {
  OpenGL->OnContextSwitch();

  vec2 canvasSize, topLeft;
  GetCanvasDimensions(canvasSize, topLeft);

  ThePainter->SetupViewport(glWidget->width(), glWidget->height(), topLeft, canvasSize);

  //glClearColor(0.26f, 0.26f, 0.26f, 1.0f);
  OpenGL->Clear(true, false, 0x434343);

  /// Draw connections
  ThePainter->mColor->Set(vec4(1, 1, 1, 1));
  for (const auto& node : GetGraph()->mNodes.GetDirectMultiNodes()) {
    std::shared_ptr<NodeWidget> nodeWidget = mWidgetMap.at(node);
    auto& widgetSlots = nodeWidget->GetWidgetSlots();
    for (UINT i = 0; i < widgetSlots.size(); i++) {
      Slot* slot = widgetSlots[i]->mSlot;
      if (slot->mIsMultiSlot) {
        for (const auto& connectedNode : slot->GetDirectMultiNodes()) {
          std::shared_ptr<NodeWidget> connectedNodeWidget = GetNodeWidget(connectedNode);
          if (connectedNodeWidget != nullptr) {
            /// Draw connection
            const vec2 p1 = connectedNodeWidget->GetOutputPosition();
            const vec2 p2 = nodeWidget->GetInputPosition(i);
            ThePainter->DrawLine(p1.x, p1.y, p2.x, p2.y);
          }
        }
      }
      else {
        /// TODO: remove code duplication
        const auto& connectedNode = slot->GetDirectNode();
        if (connectedNode) {
          std::shared_ptr<NodeWidget> connectedNodeWidget = GetNodeWidget(connectedNode);
          if (connectedNodeWidget) {
            /// Draw connection
            const vec2 p1 = connectedNodeWidget->GetOutputPosition();
            const vec2 p2 = nodeWidget->GetInputPosition(i);
            ThePainter->DrawLine(p1.x, p1.y, p2.x, p2.y);
          }
        }
      }
    }
  }

  if (mCurrentState == State::CONNECT_TO_NODE) {
    const vec2 from = mClickedWidget->GetInputPosition(mClickedSlotIndex);
    const vec2 to = mCurrentMousePos;
    ThePainter->mColor->Set(vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  if (mCurrentState == State::CONNECT_TO_SLOT) {
    const vec2 from = mClickedWidget->GetOutputPosition();
    const vec2 to = mCurrentMousePos;
    ThePainter->mColor->Set(vec4(1, 1, 1, 0.7));
    ThePainter->DrawLine(from, to);
  }

  /// Draw nodes
  for (const auto& node : GetGraph()->mNodes.GetDirectMultiNodes()) {
    mWidgetMap.at(node)->Paint();
  }

  /// Draw selection rectangle
  if (mCurrentState == State::SELECT_RECTANGLE) {
    ThePainter->mColor->Set(vec4(0.4, 0.9, 1, 0.1));
    ThePainter->DrawBox(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
    ThePainter->mColor->Set(vec4(0.6, 0.9, 1, 0.6));
    ThePainter->DrawRect(mOriginalMousePos, mCurrentMousePos - mOriginalMousePos);
  }

  mRedrawRequested = false;
}


void GraphWatcher::Update() const
{
  GetGlWidget()->update();
}


void GraphWatcher::HandleMousePress(EventForwarderGlWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftDown(event);
  }
  else if (event->button() == Qt::RightButton) {
    HandleMouseRightDown(event);
  }
}


void GraphWatcher::HandleMouseRelease(EventForwarderGlWidget*, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    HandleMouseLeftUp(event);
  }
  else if (event->button() == Qt::RightButton) {
    HandleMouseRightUp(event);
  }
}


std::shared_ptr<NodeWidget> GraphWatcher::GetNodeWidget(const std::shared_ptr<Node>& node) {
  const auto it = mWidgetMap.find(node);
  return (it != mWidgetMap.end()) ? it->second : nullptr;
}


void GraphWatcher::SetWatcherWidget(WatcherWidget* watcherWidget) {
  WatcherUi::SetWatcherWidget(watcherWidget);

  watcherWidget->setAcceptDrops(true);
  GetGlWidget()->setMouseTracking(true);
  GetGlWidget()->setFocusPolicy(Qt::ClickFocus);

  GetGlWidget()->makeCurrent();
  const auto graph = PointerCast<Graph>(GetNode());
  for (const auto& node : graph->mNodes.GetDirectMultiNodes()) {
    const std::shared_ptr<NodeWidget> widget = node->Watch<NodeWidget>(node,
      std::bind(&GraphWatcher::HandleNodeWidgetRedraw, this));
    mWidgetMap[node] = widget;
  }

  GetGlWidget()->mOnPaint += Delegate(this, &GraphWatcher::Paint);
  GetGlWidget()->mOnMousePress += Delegate(this, &GraphWatcher::HandleMousePress);
  GetGlWidget()->mOnMouseRelease += Delegate(this, &GraphWatcher::HandleMouseRelease);
  GetGlWidget()->mOnMouseMove += Delegate(this, &GraphWatcher::HandleMouseMove);
  GetGlWidget()->mOnKeyPress += Delegate(this, &GraphWatcher::HandleKeyPress);
  GetGlWidget()->mOnMouseWheel += Delegate(this, &GraphWatcher::HandleMouseWheel);
}

bool IsInsideRect(vec2 position, vec2 topleft, vec2 size) {
  return (position.x >= topleft.x && position.x <= topleft.x + size.x
    && position.y >= topleft.y && position.y <= topleft.y + size.y);
}


bool HasIntersection(vec2 pos1, vec2 size1, vec2 pos2, vec2 size2) {
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
  for (const std::shared_ptr<NodeWidget>& widget : mSelectedNodeWidgets) {
    widget->SetSelected(false);
  }
  mSelectedNodeWidgets.clear();
}


void GraphWatcher::SelectSingleWidget(const std::shared_ptr<NodeWidget>& nodeWidget) {
  for (const auto& it : mWidgetMap) {
    it.second->SetSelected(it.second == nodeWidget);
  }
  mSelectedNodeWidgets.clear();
  mSelectedNodeWidgets.insert(nodeWidget);
  ZenGarden::GetInstance()->SetNodeForPropertyEditor(nodeWidget->GetDirectNode());
}

void GraphWatcher::StorePositionOfSelectedNodes() {
  mOriginalWidgetPositions.clear();
  for (const std::shared_ptr<NodeWidget>& nodeWidget : mSelectedNodeWidgets) {
    mOriginalWidgetPositions[nodeWidget] = nodeWidget->GetDirectNode()->GetPosition();
  }
}

void GraphWatcher::HandleMouseLeftDown(QMouseEvent* event) {
  const vec2 mousePos = MouseToWorld(event);
  mOriginalMousePos = mousePos;
  mCurrentMousePos = mousePos;

  switch (mCurrentState) {
  case State::DEFAULT:
    if (mHoveredWidget) {
      if ((event->modifiers() & Qt::ShiftModifier) > 0) {
        mClickedWidget = mHoveredWidget;
        mClickedSlotIndex = mHoveredSlotIndex;
        mIsConnectionValid = false;
        DeselectAll();
        mCurrentState = 
          mHoveredSlotIndex < 0 ? State::CONNECT_TO_SLOT : State::CONNECT_TO_NODE;
      }
      else {
        if (mSelectedNodeWidgets.find(mHoveredWidget) == mSelectedNodeWidgets.end()) {
          /// Select node
          SelectSingleWidget(mHoveredWidget);
        }

        StorePositionOfSelectedNodes();
        mAreNodesMoved = false;
        mClickedWidget = mHoveredWidget;
        mCurrentState = State::MOVE_NODES;

        /// Put node on top
        GetGraph()->mNodes.ChangeNodeIndex(mHoveredWidget->GetDirectNode(), 0);
      }
    }
    else {
      /// No widget was pressed, start rectangular selection
      mCurrentState = State::SELECT_RECTANGLE;
      DeselectAll();
      ZenGarden::GetInstance()->SetNodeForPropertyEditor(nullptr);
    }
    break;
  case State::CONNECT_TO_NODE:
    break;
  default: break;
  }
  GetGlWidget()->update();
}


void GraphWatcher::HandleMouseLeftUp(QMouseEvent* event) {
  switch (mCurrentState) {
  case State::MOVE_NODES:
    if (mAreNodesMoved) {
      for (const std::shared_ptr<NodeWidget>& widget : mSelectedNodeWidgets) {
        vec2 pos = widget->GetDirectNode()->GetPosition();
        TheCommandStack->Execute(
          new MoveNodeCommand(widget->GetDirectNode(), pos,
            mOriginalWidgetPositions.at(widget)));
      }
    }
    else {
      SelectSingleWidget(mClickedWidget);
      GetGlWidget()->update();
    }
    mCurrentState = State::DEFAULT;
    break;
  case State::SELECT_RECTANGLE:
    for (const auto& node : GetGraph()->mNodes.GetDirectMultiNodes()) {
      std::shared_ptr<NodeWidget> widget = mWidgetMap.at(node);
      if (widget->IsSelected()) mSelectedNodeWidgets.insert(widget);
    }
    mCurrentState = State::DEFAULT;
    GetGlWidget()->update();
    break;
  case State::CONNECT_TO_NODE:
    if (mIsConnectionValid) {
      const std::shared_ptr<Node> node = mHoveredWidget->GetDirectNode();
      Slot* slot = mClickedWidget->GetWidgetSlots()[mClickedSlotIndex]->mSlot;
      TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
    }
    GetGlWidget()->update();
    mCurrentState = State::DEFAULT;
    break;
  case State::CONNECT_TO_SLOT:
    if (mIsConnectionValid) {
      const std::shared_ptr<Node> node = mClickedWidget->GetDirectNode();
      Slot* slot = mHoveredWidget->GetWidgetSlots()[mHoveredSlotIndex]->mSlot;
      TheCommandStack->Execute(new ConnectNodeToSlotCommand(node, slot));
    }
    GetGlWidget()->update();
    mCurrentState = State::DEFAULT;
    break;
  default: break;
  }
}


void GraphWatcher::HandleMouseRightDown(QMouseEvent* event) {
  if ((event->modifiers() & Qt::ShiftModifier) > 0) {
    if (mHoveredSlotIndex >= 0) {
      /// Remove connection
      Slot* slot = mHoveredWidget->GetWidgetSlots()[mHoveredSlotIndex]->mSlot;
      if (slot->mIsMultiSlot) {
        /// HACK HACK HACK
        slot->DisconnectAll(true);
      }
      else if (slot->GetReferencedNode()) {
        TheCommandStack->Execute(new ConnectNodeToSlotCommand(nullptr, slot));
      }
    }
    return;
  }

  /// Pan canvas
  if (mCurrentState == State::DEFAULT) {
    mOriginalMousePos = vec2(float(event->x()), float(event->y()));
    mOriginalCenter = mCenter;
    mCurrentState = State::PAN_CANVAS;
  }
}


void GraphWatcher::HandleMouseRightUp(QMouseEvent* event) {
  if (mCurrentState == State::PAN_CANVAS) {
    mCurrentState = State::DEFAULT;
  }
}


void GraphWatcher::HandleMouseMove(EventForwarderGlWidget*, QMouseEvent* event) {
  const vec2 mousePos = MouseToWorld(event);
  mCurrentMousePos = mousePos;

  /// Reset all frame colors
  for (auto& it : mWidgetMap) {
    std::shared_ptr<NodeWidget> widget = it.second;
    widget->SetFrameColor(NodeWidget::ColorState::DEFAULT);
    widget->SetSlotColor(-1, NodeWidget::ColorState::DEFAULT);
  }

  FindHoveredWidget(mousePos, &mHoveredWidget, &mHoveredSlotIndex);

  switch (mCurrentState) {
  case State::MOVE_NODES:
  {
    const vec2 mouseDiff = mousePos - mOriginalMousePos;
    if ((event->modifiers() & Qt::ControlModifier) > 0 && !mAreNodesMoved) {
      const std::shared_ptr<Node> originalNode = mHoveredWidget->GetDirectNode();
      std::shared_ptr<Ghost> ghost = std::make_shared<Ghost>();
      ghost->mOriginalNode.Connect(originalNode);
      ghost->SetName(originalNode->GetName());
      const vec2 position = mOriginalWidgetPositions[mHoveredWidget] + mouseDiff;
      ghost->SetPosition(position);
      TheCommandStack->Execute(new CreateNodeCommand(ghost, GetGraph()));
      /// Adding new nodes to a graph resets watcher state, but we want to move the ghost
      mCurrentState = State::MOVE_NODES;
      const std::shared_ptr<NodeWidget> ghostWidget = mWidgetMap.at(ghost);
      SelectSingleWidget(ghostWidget);
      StorePositionOfSelectedNodes();
      mAreNodesMoved = true;
      mOriginalMousePos = mousePos;
      return;
    }

    mAreNodesMoved = true;
    for (const std::shared_ptr<NodeWidget>& widget : mSelectedNodeWidgets) {
      widget->GetDirectNode()->SetPosition(mOriginalWidgetPositions[widget] + mouseDiff);
    }
    GetGlWidget()->update();
  }
  break;
  case State::SELECT_RECTANGLE:
    for (const auto& node : GetGraph()->mNodes.GetDirectMultiNodes()) {
      std::shared_ptr<NodeWidget> widget = mWidgetMap.at(node);
      const bool selected = HasIntersection(mOriginalMousePos,
        mCurrentMousePos - mOriginalMousePos, node->GetPosition(), node->GetSize());
      widget->SetSelected(selected);
    }
    GetGlWidget()->update();
    break;
  case State::CONNECT_TO_NODE:
    mIsConnectionValid = false;
    if (mHoveredWidget && mHoveredWidget != mClickedWidget) {
      /// TODO: check for graph cycles
      mIsConnectionValid =
        mClickedWidget->GetWidgetSlots()[mClickedSlotIndex]->mSlot->DoesAcceptNode(
          mHoveredWidget->GetDirectNode());
      mHoveredWidget->SetFrameColor(mIsConnectionValid
        ? NodeWidget::ColorState::VALID_CONNECTION
        : NodeWidget::ColorState::INVALID_CONNECTION);
    }
    GetGlWidget()->update();
    break;
  case State::CONNECT_TO_SLOT:
    mIsConnectionValid = false;
    if (mHoveredWidget) {
      mHoveredWidget->SetFrameColor(NodeWidget::ColorState::HOVERED);
      if (mHoveredSlotIndex >= 0 && mHoveredWidget != mClickedWidget) {
        /// TODO: check for graph cycles
        mIsConnectionValid = (mHoveredWidget->GetWidgetSlots()[mHoveredSlotIndex]->mSlot->DoesAcceptNode(
          mClickedWidget->GetDirectNode()));
        mHoveredWidget->SetSlotColor(mHoveredSlotIndex, mIsConnectionValid
          ? NodeWidget::ColorState::VALID_CONNECTION
          : NodeWidget::ColorState::INVALID_CONNECTION);
      }
    }
    GetGlWidget()->update();
    break;
  case State::PAN_CANVAS:
  {
    const vec2 mousePixelPos(float(event->x()), float(event->y()));
    const vec2 diff = mousePixelPos - mOriginalMousePos;
    mCenter = mOriginalCenter - diff * mZoomFactor;
    GetGlWidget()->update();
  }
  break;
  default:
    if (mHoveredWidget) {
      mHoveredWidget->SetFrameColor(NodeWidget::ColorState::HOVERED);
      mHoveredWidget->SetSlotColor(mHoveredSlotIndex, NodeWidget::ColorState::HOVERED);
    }
    if (NeedsWidgetRepaint()) {
      GetGlWidget()->update();
    }
    break;
  }
}


void GraphWatcher::HandleMouseWheel(EventForwarderGlWidget*, QWheelEvent* event) {
  mZoomExponent -= event->delta();
  if (mZoomExponent < 0) mZoomExponent = 0;
  mZoomFactor = powf(2.0, float(mZoomExponent) / (120.0f * 4.0f));
  GetGlWidget()->update();
}


void GraphWatcher::FindHoveredWidget(vec2 mousePos,
  std::shared_ptr<NodeWidget>* oHoveredWidget, int* oSlotIndex)
{
  *oHoveredWidget = nullptr;
  *oSlotIndex = -1;

  for (auto& it : mWidgetMap) {
    const std::shared_ptr<Node> node = it.first;
    const std::shared_ptr<NodeWidget> widget = it.second;
    if (IsInsideRect(mousePos, node->GetPosition(), node->GetSize())) {
      *oHoveredWidget = widget;
      auto& widgetSlots = widget->GetWidgetSlots();
      for (UINT o = 0; o < widgetSlots.size(); o++) {
        NodeWidget::WidgetSlot* sw = widgetSlots[o];
        if (IsInsideRect(mousePos, node->GetPosition() + sw->mPosition, sw->mSize)) {
          *oSlotIndex = o;
          break;
        }
      }
      return;
    }
  }
}


void GraphWatcher::HandleKeyPress(EventForwarderGlWidget*, QKeyEvent* event) {
  const auto scanCode = event->nativeScanCode();

  if (scanCode == 41) {
    /// The key over "tab", independent of input language
    /// Open "add new node" window
    const std::shared_ptr<Node> node = ThePrototypes->AskUser(mWatcherWidget, QCursor::pos());
    if (node) {
      TheCommandStack->Execute(new CreateNodeCommand(node, GetGraph()));
      TheCommandStack->Execute(new MoveNodeCommand(node, mCurrentMousePos));
    }
    return;
  }

  switch (event->key()) {
  case Qt::Key_Delete:
    if (!mSelectedNodeWidgets.empty()) {
      std::set<std::shared_ptr<Node>> selectedNodes;
      for (const std::shared_ptr<NodeWidget>& nodeWidget : mSelectedNodeWidgets) {
        selectedNodes.insert(nodeWidget->GetDirectNode());
      }
      Util::DisposeNodes(selectedNodes);
      ZenGarden::GetInstance()->SetNodeForPropertyEditor(nullptr);
    }
    break;

    /// 1 opens watcher on upper left panel
  case Qt::Key_1:
    if (mSelectedNodeWidgets.size() == 1) {
      ZenGarden::GetInstance()->Watch((*mSelectedNodeWidgets.begin())->GetDirectNode(),
        WatcherPosition::UPPER_LEFT_TAB);
    }
    break;

    /// 2 opens watcher on right panel
  case Qt::Key_2:
    if (mSelectedNodeWidgets.size() == 1) {
      ZenGarden::GetInstance()->Watch(
        (*mSelectedNodeWidgets.begin())->GetDirectNode(), WatcherPosition::RIGHT_TAB);
    }
    break;

    /// 3 opens watcher on bottom left panel
  case Qt::Key_3:
    if (mSelectedNodeWidgets.size() == 1) {
      ZenGarden::GetInstance()->Watch((*mSelectedNodeWidgets.begin())->GetDirectNode(),
        WatcherPosition::BOTTOM_LEFT_TAB);
    }
    break;
  case Qt::Key_G:
  {
    if (mCurrentState != State::DEFAULT) return;
    const std::shared_ptr<Node> originalNode = ZenGarden::GetInstance()->GetNodeInPropertyEditor();
    if (!originalNode) return;
    if (IsPointerOf<Graph>(originalNode)) return;
    std::shared_ptr<Ghost> ghost = std::make_shared<Ghost>();
    ghost->mOriginalNode.Connect(originalNode);
    ghost->SetPosition(mCurrentMousePos);
    ghost->SetName(originalNode->GetName());
    TheCommandStack->Execute(new CreateNodeCommand(ghost, GetGraph()));
    /// Adding new nodes to a graph resets watcher state, but we want to move the ghost
    const std::shared_ptr<NodeWidget> ghostWidget = mWidgetMap.at(ghost);
    SelectSingleWidget(ghostWidget);
    StorePositionOfSelectedNodes();
    break;
  }
  default: break;
  }
}


std::shared_ptr<Graph> GraphWatcher::GetGraph() const
{
  return PointerCast<Graph>(GetNode());
}


void GraphWatcher::OnSlotConnectionChanged(Slot* slot) {
  bool changed = false;

  /// Create widgets for newly added nodes
  for (const auto& node : GetGraph()->mNodes.GetDirectMultiNodes()) {
    auto it = mWidgetMap.find(node);
    if (it == mWidgetMap.end()) {
      const std::shared_ptr<NodeWidget> widget = node->Watch<NodeWidget>(node,
        std::bind(&GraphWatcher::HandleNodeWidgetRedraw, this));
      mWidgetMap[node] = widget;
      changed = true;
    }
  }

  /// Remove widgets of removed nodes
  auto& nodes = GetGraph()->mNodes.GetDirectMultiNodes();
  while (true) {
    bool found = false;
    for (const auto& it : mWidgetMap) {
      std::shared_ptr<Node> node = it.first;
      if (find(nodes.begin(), nodes.end(), node) == nodes.end()) {
        std::shared_ptr<NodeWidget> widget = it.second;
        mWidgetMap.erase(node);
        found = true;
        changed = true;
        break;
      }
    }
    if (!found) break;
  }

  if (changed) {
    DeselectAll();
    mHoveredWidget.reset();
    mClickedWidget.reset();
    mCurrentState = State::DEFAULT;
    GetGlWidget()->update();
  }
}


vec2 GraphWatcher::CanvasToWorld(const vec2& canvasCoord) const {
  vec2 canvasSize, topLeft;
  GetCanvasDimensions(canvasSize, topLeft);
  return topLeft + canvasCoord * mZoomFactor;
}


vec2 GraphWatcher::MouseToWorld(QMouseEvent* event) const {
  const vec2 mouseCoord(float(event->x()), float(event->y()));
  return CanvasToWorld(mouseCoord);
}

void GraphWatcher::GetCanvasDimensions(vec2& oCanvasSize, vec2& oTopLeft) const
{
  oCanvasSize = vec2(float(GetGlWidget()->width()), float(GetGlWidget()->height())) * 
    mZoomFactor;
  oTopLeft = mCenter - oCanvasSize / 2.0f;

  /// Consistent shape of nodes
  oTopLeft.x = floorf(oTopLeft.x);
  oTopLeft.y = floorf(oTopLeft.y);
}


void GraphWatcher::HandleDragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* mimeData = event->mimeData();
  if (!mimeData->hasUrls()) return;
  const QList<QUrl> urlList = mimeData->urls();
  if (urlList.size() != 1) return;

  const QString fileName = urlList.at(0).toLocalFile();
  if (fileName.endsWith(".obj") || fileName.endsWith(".3ds") ||
    fileName.endsWith(".png") || fileName.endsWith(".jpg") ||
    fileName.endsWith(".fbx")) {
    event->acceptProposedAction();
  }
}

/// TODO: remove code duplication with HandleDragEnterEvent
void GraphWatcher::HandleDropEvent(QDropEvent* event) {
  const QMimeData* mimeData = event->mimeData();
  if (!mimeData->hasUrls()) return;
  const QList<QUrl> urlList = mimeData->urls();
  if (urlList.size() != 1) return;

  const QString absolutefileName = urlList.at(0).toLocalFile();
  const QString fileName = QDir::current().relativeFilePath(absolutefileName);

  const QFileInfo fileInfo(fileName);
  if (fileInfo.suffix() == "obj" || fileInfo.suffix() == "3ds" ||
    fileInfo.suffix() == "fbx") {
    const std::shared_ptr<Mesh> mesh = Util::LoadMesh(fileName);
    std::shared_ptr<StaticMeshNode> node = std::make_shared<StaticMeshNode>();
    node->Set(mesh);
    node->SetName(fileInfo.fileName().toStdString());
    TheCommandStack->Execute(new CreateNodeCommand(node, GetGraph()));

    const vec2 pos = 
      CanvasToWorld(vec2(float(event->pos().x()), float(event->pos().y())));
    TheCommandStack->Execute(new MoveNodeCommand(node, pos));
    event->acceptProposedAction();
  }

  else if (fileInfo.suffix() == "png" || fileInfo.suffix() == "jpg") {
    auto node = std::make_shared<TextureFileNode>();
    node->mFileName.SetDefaultValue(fileName.toStdString());
    node->SetName(fileInfo.baseName().toStdString());
    TheCommandStack->Execute(new CreateNodeCommand(node, GetGraph()));
    const vec2 pos = 
      CanvasToWorld(vec2(float(event->pos().x()), float(event->pos().y())));
    TheCommandStack->Execute(new MoveNodeCommand(node, pos));
    event->acceptProposedAction();
  }
}

void GraphWatcher::HandleNodeWidgetRedraw() {
  if (!mRedrawRequested) {
    mRedrawRequested = true;
    Update();
  }
}

bool GraphWatcher::NeedsWidgetRepaint() {
  for (auto& it : mWidgetMap) {
    if (it.second->NeedsRepaint()) return true;
  }
  return false;
}
