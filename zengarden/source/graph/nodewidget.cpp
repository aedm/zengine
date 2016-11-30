#include "nodewidget.h"
#include "prototypes.h"
#include "../util/uipainter.h"
#include <zengine.h>
#include <QImage>
#include <QGLWidget>
#include <QPixmap>
#include <QPainter>

/// Layout of the widget

/// Padding of the title text
static const float TitlePadding = ADJUST(3.0f);

/// Space between slots
static const float SlotSpacing = ADJUST(5.0f);

/// Space between slots
static const Vec2 SlotPadding = ADJUST(Vec2(10.0f, 1.0f));

/// Left margin in front of slots
static const float SlotLeftMargin = ADJUST(5.0f);

/// Left margin in front of slots
static const float SlotRightMargin = ADJUST(20.0f);

/// Slot width
static const float SlotWidth = ADJUST(80.0f);

static const Vec2 ConnectionSpotSize = ADJUST(Vec2(4.0f, 4.0f));
static const float ConnectionSpotPadding = ADJUST(8.0f);

static const float Opacity = 0.7f;

static const Vec4 ConnectionColor(1, 0.3, 1, 1);
static const Vec4 ConnectionColorValid(0, 1, 0, 1);
static const Vec4 ConnectionColorInvalid(1, 0, 0, 1);


NodeWidget::NodeWidget(Node* node, GraphWatcher* graphWatcher)
	: WatcherUI(node, nullptr, NodeType::WIDGET)
	, mTitleTexture(nullptr)
  , mGraphWatcher(graphWatcher)
{
	mIsSelected = false;
  OnNameChange();
	CreateWidgetSlots();
}


NodeWidget::~NodeWidget() {
  SafeDelete(mTitleTexture);
}


void NodeWidget::CreateWidgetSlots()
{
	for (auto x : mWidgetSlots) delete x;
	mWidgetSlots.clear();

  for (Slot* slot : mNode->GetPublicSlots())
	{
    if (slot->DoesAcceptType(NodeType::STRING)) continue;
		WidgetSlot* sw = new WidgetSlot();
		sw->mTexture.SetText(QString::fromStdString(*slot->GetName()), ThePainter->mTitleFont);
		sw->mSlot = slot;
		mWidgetSlots.push_back(sw);
	}
	CalculateLayout();
}


void NodeWidget::CalculateLayout()
{
	mTitleHeight = mTitleTexture->TextSize.height() + TitlePadding * 2.0f + 1.0f;
	float slotY = mTitleHeight + SlotSpacing;
	for(WidgetSlot* sw : mWidgetSlots)
	{
		sw->mPosition = Vec2(SlotLeftMargin, slotY);
		sw->mSize = 
      Vec2(SlotWidth, float(sw->mTexture.TextSize.height()) + SlotPadding.y * 2.0f);
		sw->mSpotPos = Vec2(ConnectionSpotPadding, slotY + sw->mSize.y / 2.0f);
		slotY += sw->mSize.y + SlotSpacing;
	}
	Vec2 size(SlotLeftMargin + SlotWidth + SlotRightMargin, slotY + 1);
	mOutputPosition = Vec2(size.x - ConnectionSpotPadding - 1.0f, mTitleHeight / 2.0f);
  mNode->SetSize(size);
}


void NodeWidget::Paint()
{
  Vec2 position = mNode->GetPosition();
  Vec2 size = mNode->GetSize();

  ThePainter->mColor.Set(Vec4(0, 0.2, 0.4, Opacity));
	ThePainter->DrawBox(position, Vec2(size.x, mTitleHeight));

	ThePainter->mColor.Set(Vec4(0, 0, 0, Opacity));
	ThePainter->DrawBox(position + Vec2(0, mTitleHeight), size - Vec2(0, mTitleHeight));
	
	ThePainter->mColor.Set(Vec4(0.2, 0.7, 0.9, 1));
	ThePainter->DrawBox(position + mOutputPosition - ConnectionSpotSize * 0.5f, 
                      ConnectionSpotSize);
	
	ThePainter->mColor.Set(Vec4(0.9, 0.9, 0.9, 1));
	float centerX = floor((size.x - float(mTitleTexture->TextSize.width())) * 0.5f);
	ThePainter->DrawTextTexture(mTitleTexture, position + Vec2(centerX, TitlePadding + 1));

  /// Paint slots
	for (int i=0; i<mWidgetSlots.size(); i++)
	{
		Vec4 slotFrameColor(1, 1, 1, 0.1);
		if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
      if (mGraphWatcher->mClickedWidget == this 
          && mGraphWatcher->mClickedSlotIndex == i) {
				slotFrameColor = ConnectionColor;
			}
    } else if (mGraphWatcher->mHoveredWidget == this 
               && mGraphWatcher->mHoveredSlotIndex == i) {
      if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT) {
        slotFrameColor = mGraphWatcher->mIsConnectionValid
					? ConnectionColorValid : ConnectionColorInvalid;
			} else slotFrameColor = Vec4(1, 1, 1, 0.2);
		}

		WidgetSlot* sw = mWidgetSlots[i];
		ThePainter->mColor.Set(slotFrameColor);
		ThePainter->DrawRect(position + sw->mPosition, sw->mSize);

		ThePainter->mColor.Set(Vec4(0.9, 0.9, 0.9, 1));
		ThePainter->DrawTextTexture(&sw->mTexture, position + sw->mPosition + SlotPadding);

		ThePainter->mColor.Set(Vec4(0.2, 0.7, 0.9, 1));
		ThePainter->DrawBox(position + sw->mSpotPos - ConnectionSpotSize * 0.5f, 
                        ConnectionSpotSize);
	}
	
  /// Paint frame
	Vec4 frameColor(1, 1, 1, 0.1);
	if (mIsSelected) {
		frameColor = Vec4(1, 1, 1, 1);
  } else if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT
             && mGraphWatcher->mClickedWidget == this) {
			frameColor = ConnectionColor;
  } else if (mGraphWatcher->mHoveredWidget == this) {
    if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
      if (mGraphWatcher->mClickedWidget != this) {
        frameColor = mGraphWatcher->mIsConnectionValid
          ? ConnectionColorValid : ConnectionColorInvalid;
			}
		} else frameColor = Vec4(1, 1, 1, 0.3);
	} 
	ThePainter->mColor.Set(frameColor);
	ThePainter->DrawRect(position, size);
}


Vec2 NodeWidget::GetOutputPosition()
{
  return mNode->GetPosition() + mOutputPosition;
}


Vec2 NodeWidget::GetInputPosition( int SlotIndex )
{
	WidgetSlot* sw = mWidgetSlots[SlotIndex];
  return mNode->GetPosition() + sw->mSpotPos;
}


void NodeWidget::OnSlotStructureChanged() {
  CreateWidgetSlots();
  mGraphWatcher->Update();
}


void NodeWidget::OnNameChange() {
  static const QString stubLabel(" [stub]");

  QString text;
  if (!mNode->GetName().empty()) {
    /// Node has a name, use that.
    text = QString::fromStdString(mNode->GetName());
  } else {
    /// Just use the type as a name by default
    text = QString::fromStdString(
      NodeRegistry::GetInstance()->GetNodeClass(mNode)->mClassName);
    if (mNode->GetType() == NodeType::SHADER_STUB) {
      StubNode* stub = static_cast<StubNode*>(mNode);
      StubMetadata* metaData = stub->GetStubMetadata();
      if (metaData != nullptr && !metaData->name.empty()) {
        /// For shader stubs, use the stub name by default
        text = QString::fromStdString(metaData->name);
      }
    }
  }
  mTitleTexture = new TextTexture();
  mTitleTexture->SetText(text, ThePainter->mTitleFont);
  mGraphWatcher->Update();
}


void NodeWidget::OnGraphPositionChanged() {
  mGraphWatcher->Update();
}


void NodeWidget::OnSlotConnectionChanged(Slot* slot) {
  mGraphWatcher->Update();
}
