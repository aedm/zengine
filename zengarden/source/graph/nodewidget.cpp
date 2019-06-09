#include "nodewidget.h"
#include "prototypes.h"
#include "../util/uipainter.h"
#include "../zengarden.h"
#include <zengine.h>
#include <QImage>
#include <QGLWidget>
#include <QPixmap>
#include <QGuiApplication>
#include <QScreen>

/// Layout of the widget

/// Padding of the title text
static const float TitlePadding = ADJUST(3.0f);

/// Space between slots
static const float SlotSpacing = ADJUST(5.0f);

/// Space between slots
static const Vec2 SlotPadding = ADJUST(Vec2(5.0f, 1.0f));

/// Left margin in front of slots
static const float SlotLeftMargin = ADJUST(5.0f);

/// Left margin in front of slots
static const float SlotRightMargin = ADJUST(20.0f);

/// Slot width
static const float SlotWidth = ADJUST(80.0f);

static const Vec2 ConnectionSpotSize = ADJUST(Vec2(4.0f, 4.0f));
static const float ConnectionSpotPadding = ADJUST(8.0f);

static const float Opacity = 0.7f;

struct {
  const QColor mSlotDefault = QColor::fromRgbF(1, 1, 1, 0.1);
  const QColor mSlotHovered = QColor::fromRgbF(1, 1, 1, 0.2);
  const QColor mSlotConnectionFrom = QColor::fromRgbF(1, 0.3, 1, 1);
  const QColor mSlotValidConnection = QColor::fromRgbF(0, 1, 0, 1);
  const QColor mSlotInvalidConnection = QColor::fromRgbF(1, 0, 0, 1);
  const QColor mFrameDefault = QColor::fromRgbF(1, 1, 1, 0.1);
  const QColor mFrameSelected = QColor::fromRgbF(1, 1, 1, 1);
  const QColor mFrameHovered = QColor::fromRgbF(1, 1, 1, 0.3);
  const QColor mFrameConnectionFrom = QColor::fromRgbF(1, 0.3, 1, 1);
  const QColor mFrameValidConnection = QColor::fromRgbF(0, 1, 0, 1);
  const QColor mFrameInvalidConnection = QColor::fromRgbF(1, 0, 0, 1);
} Colors;

NodeWidget::NodeWidget(const shared_ptr<Node>& node,
  const std::function<void()>& onNeedsRedraw)
  : WatcherUI(node)
  , mOnNeedsRedraw(onNeedsRedraw)
{
  OnNameChange();
  CreateWidgetSlots();
}


NodeWidget::~NodeWidget() {
  DiscardTexture();
}

void NodeWidget::SetFrameColor(NodeWidget::ColorState frameColorState) {
  mNextPaintState.mFrameColorState = frameColorState;
}

void NodeWidget::SetSlotColor(int slotIndex, NodeWidget::ColorState slotColorState) {
  mNextPaintState.mSlotColorState = slotColorState;
  mNextPaintState.mColoredSlotIndex = slotIndex;
}

void NodeWidget::SetSelected(bool isSelected) {
  mNextPaintState.mIsSelected = isSelected;
}

bool NodeWidget::IsSelected() {
  return mNextPaintState.mIsSelected;
}

bool NodeWidget::NeedsRepaint() {
  return mForceUpdate ||
    mCurrentPaintState.mFrameColorState != mNextPaintState.mFrameColorState ||
    mCurrentPaintState.mSlotColorState != mNextPaintState.mSlotColorState ||
    mCurrentPaintState.mColoredSlotIndex != mNextPaintState.mColoredSlotIndex ||
    mCurrentPaintState.mIsSelected != mNextPaintState.mIsSelected ||
    mCurrentPaintState.mHighlightedSlotIndex != mNextPaintState.mHighlightedSlotIndex;
}

void NodeWidget::CreateWidgetSlots() {
  for (auto x : mWidgetSlots) delete x;
  mWidgetSlots.clear();

  for (Slot* slot : GetDirectNode()->GetPublicSlots()) {
    if (IsInsanceOf<StringSlot*>(slot)) continue;
    WidgetSlot* sw = new WidgetSlot();
    sw->mSlot = slot;
    mWidgetSlots.push_back(sw);
  }
  CalculateLayout();
}


void NodeWidget::CalculateLayout() {
  const QFont& font = mPainter.font();
  double dpi = QGuiApplication::primaryScreen()->physicalDotsPerInch();
  const double fontHeightPoints = font.pointSizeF();
  const int fontHeight = int(fontHeightPoints / 72.0 * dpi);

  mTitleHeight = fontHeight + TitlePadding * 2.0f + 1.0f;
  float slotY = mTitleHeight + SlotSpacing;
  for (WidgetSlot* sw : mWidgetSlots) {
    sw->mPosition = Vec2(SlotLeftMargin, slotY);
    sw->mSize =
      Vec2(SlotWidth, float(fontHeight) + SlotPadding.y * 2.0f);
    sw->mSpotPos = Vec2(ConnectionSpotPadding, slotY + sw->mSize.y / 2.0f);
    slotY += sw->mSize.y + SlotSpacing;
  }
  Vec2 size(SlotLeftMargin + SlotWidth + SlotRightMargin, slotY + 1);
  mOutputPosition = Vec2(size.x - ConnectionSpotPadding - 1.0f, mTitleHeight / 2.0f);
  GetDirectNode()->SetSize(size);

  mForceUpdate = true;
}


void NodeWidget::UpdateTexture() {
  if (NeedsRepaint()) {
    PaintToImage();
    /// This makes a copy of the array
    unsigned char* bits = mImage.bits();
    int height = mImage.height();
    int width = mImage.width();
    shared_ptr<vector<char>> texels = make_shared<vector<char>>(height * width * 4);
    unsigned char* source = bits;
    unsigned char* dest = reinterpret_cast<unsigned char*>(&(*texels)[0]);
    for (int i = 0; i < height * width; i++) {
      /// Swap RGBA and BGRA
      dest[0] = source[2];
      dest[1] = source[1];
      dest[2] = source[0];
      dest[3] = source[3];
      dest += 4;
      source += 4;
    }

    if (mTexture && mTexture->mWidth == mImage.width() &&
      mTexture->mHeight == mImage.height()) {
      OpenGL->UploadTextureGPUData(mTexture, &(*texels)[0]);
    }
    else {
      DiscardTexture();
      mTexture = OpenGL->MakeTexture(width, height, TexelType::ARGB8, texels, true,
        false, false, false);
    }
    mForceUpdate = false;
    mCurrentPaintState = mNextPaintState;
  }
}

static Vec4 LiveHeaderColor = Vec4(0, 0.2, 0.4, Opacity);
static Vec4 GhostHeaderColor = Vec4(0.4, 0.2, 0, Opacity);
static Vec4 ReferenceHeaderColor = Vec4(0.4, 0.0, 0.2, Opacity);


void NodeWidget::Paint() {
  UpdateTexture();

  shared_ptr<Node> node = GetDirectNode();
  Vec2 position = node->GetPosition();
  ThePainter->DrawTexture(mTexture, position.x, position.y);
}


Vec2 NodeWidget::GetOutputPosition()
{
  return GetDirectNode()->GetPosition() + mOutputPosition;
}


Vec2 NodeWidget::GetInputPosition(int SlotIndex)
{
  WidgetSlot* sw = mWidgetSlots[SlotIndex];
  return GetDirectNode()->GetPosition() + sw->mSpotPos;
}


const std::vector<NodeWidget::WidgetSlot*>& NodeWidget::GetWidgetSlots() {
  return mWidgetSlots;
}

void NodeWidget::OnSlotStructureChanged() {
  CreateWidgetSlots();
  mOnNeedsRedraw();
}


void NodeWidget::OnNameChange() {
  static const QString stubLabel(" [stub]");
  shared_ptr<Node> directNode = GetDirectNode();

  QString text;
  if (!directNode->GetName().empty()) {
    /// Node has a name, use that.
    text = QString::fromStdString(directNode->GetName());
  }
  else {
    shared_ptr<Node> node = GetNode();
    if (!node->GetName().empty()) {
      /// Live node has a name, use that.
      text = QString::fromStdString(directNode->GetName());
    }
    else {
      /// Just use the type as a name by default
      text = QString::fromStdString(
        NodeRegistry::GetInstance()->GetNodeClass(node)->mClassName);
      if (IsPointerOf<StubNode>(node)) {
        shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
        StubMetadata* metaData = stub->GetStubMetadata();
        if (metaData != nullptr && !metaData->name.empty()) {
          /// For shader stubs, use the stub name by default
          text = QString::fromStdString(metaData->name);
        }
      }
    }
  }
  mNodeTitle = text;
  mForceUpdate = true;
  mOnNeedsRedraw();
}


void NodeWidget::OnGraphPositionChanged() {
  mOnNeedsRedraw();
}


void NodeWidget::OnSlotConnectionChanged(Slot* slot) {
  mOnNeedsRedraw();
}


QColor QColorFromVec4(const Vec4& vec) {
  return QColor::fromRgbF(vec.x, vec.y, vec.z, vec.w);
}


void NodeWidget::DiscardTexture() {
  mTexture = nullptr;
}

void NodeWidget::PaintToImage()
{
  shared_ptr<Node> node = GetDirectNode();
  Vec2 size = node->GetSize();
  int iWidth = int(ceilf(size.x));
  int iHeight = int(ceilf(size.y));

  QPixmap pixmap(iWidth, iHeight);
  pixmap.fill(QColor(0, 0, 0, 1));
  mPainter.begin(&pixmap);

  Vec4 headerColor = LiveHeaderColor;
  if (node->IsGhostNode()) {
    headerColor = PointerCast<Ghost>(node)->IsDirectReference()
      ? ReferenceHeaderColor : GhostHeaderColor;
  }

  mPainter.setPen(Qt::NoPen);
  mPainter.setBrush(QColorFromVec4(headerColor));
  mPainter.drawRect(QRectF(0, 0, size.x, mTitleHeight));

  mPainter.setPen(Qt::NoPen);
  mPainter.setBrush(QColor::fromRgbF(0, 0, 0, Opacity));
  mPainter.drawRect(QRectF(0, mTitleHeight, size.x, size.y - mTitleHeight));

  mPainter.setPen(Qt::NoPen);
  mPainter.setBrush(QColor::fromRgbF(0.2, 0.7, 0.9, 1));
  Vec2 outputTopLeft = mOutputPosition - ConnectionSpotSize * 0.5f;
  mPainter.drawRect(QRectF(outputTopLeft.x, outputTopLeft.y,
    ConnectionSpotSize.x, ConnectionSpotSize.y));

  mPainter.setPen(QColor::fromRgbF(0.9, 0.9, 0.9, 1));
  mPainter.drawText(
    QRectF(0, 0, size.x - ConnectionSpotPadding - ConnectionSpotSize.x, mTitleHeight),
    Qt::AlignVCenter | Qt::AlignCenter, mNodeTitle);

  /// Paint slots
  for (int i = 0; i < mWidgetSlots.size(); i++) {
    WidgetSlot* sw = mWidgetSlots[i];

    QColor slotColor = Colors.mSlotDefault;
    if (mNextPaintState.mColoredSlotIndex == i) {
      switch (mNextPaintState.mSlotColorState) {
      case ColorState::DEFAULT:
        slotColor = Colors.mSlotDefault;
        break;
      case ColorState::HOVERED:
        slotColor = Colors.mSlotHovered;
        break;
      case ColorState::CONNECTION_FROM:
        slotColor = Colors.mSlotConnectionFrom;
        break;
      case ColorState::VALID_CONNECTION:
        slotColor = Colors.mSlotValidConnection;
        break;
      case ColorState::INVALID_CONNECTION:
        slotColor = Colors.mSlotInvalidConnection;
        break;
      }
    }
    mPainter.setPen(slotColor);
    mPainter.setBrush(Qt::NoBrush);
    mPainter.drawRect(QRectF(sw->mPosition.x, sw->mPosition.y,
      sw->mSize.x, sw->mSize.y));

    mPainter.setPen(QColor::fromRgbF(0.9, 0.9, 0.9, 1));
    mPainter.drawText(QRectF(sw->mPosition.x + SlotPadding.x, sw->mPosition.y,
      sw->mSize.x - SlotPadding.x, sw->mSize.y),
      Qt::AlignVCenter, QString::fromStdString(sw->mSlot->mName));
  }

  /// Paint frame
  QColor frameColor;
  if (mNextPaintState.mIsSelected) {
    frameColor = Colors.mFrameSelected;
  }
  else {
    switch (mNextPaintState.mFrameColorState) {
    case ColorState::DEFAULT:
      frameColor = Colors.mFrameDefault;
      break;
    case ColorState::HOVERED:
      frameColor = Colors.mFrameHovered;
      break;
    case ColorState::CONNECTION_FROM:
      frameColor = Colors.mFrameConnectionFrom;
      break;
    case ColorState::VALID_CONNECTION:
      frameColor = Colors.mFrameValidConnection;
      break;
    case ColorState::INVALID_CONNECTION:
      frameColor = Colors.mFrameInvalidConnection;
      break;
    }
  }

  mPainter.setPen(frameColor);
  mPainter.setBrush(Qt::NoBrush);
  mPainter.drawRect(QRectF(0, 0, size.x - 1, size.y - 1));

  mPainter.end();
  mImage = QGLWidget::convertToGLFormat(pixmap.toImage().mirrored(false, true));
}
