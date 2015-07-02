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


NodeWidget::NodeWidget(Node* node)
	: Watcher(node, nullptr, NodeType::WIDGET)
	, mTitleTexture(NULL)
{
	mIsSelected = false;
  HandleTitleChange();
  
	CreateWidgetSlots();
}


void NodeWidget::CreateWidgetSlots()
{
	for (auto x : mWidgetSlots) delete x;
	mWidgetSlots.clear();

	Node* node = GetNode();
	for(Slot* slot : node->mSlots)
	{
		WidgetSlot* sw = new WidgetSlot();
		sw->mTexture.SetText(QString::fromStdString(*slot->GetName()), ThePainter->TitleFont);
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
		sw->mSize = Vec2(SlotWidth, float(sw->mTexture.TextSize.height()) + SlotPadding.y * 2.0f);
		sw->mSpotPos = Vec2(ConnectionSpotPadding, slotY + sw->mSize.y / 2.0f);
		slotY += sw->mSize.y + SlotSpacing;
	}
	mSize = Vec2(SlotLeftMargin + SlotWidth + SlotRightMargin, slotY + 1);
	mOutputPosition = Vec2(mSize.x - ConnectionSpotPadding - 1.0f, mTitleHeight / 2.0f);
}


void NodeWidget::Paint(GraphWatcher* Panel)
{
	ThePainter->Color.Set(Vec4(0, 0.2, 0.4, Opacity));
	ThePainter->DrawBox(mPosition, Vec2(mSize.x, mTitleHeight));

	ThePainter->Color.Set(Vec4(0, 0, 0, Opacity));
	ThePainter->DrawBox(mPosition + Vec2(0, mTitleHeight), mSize - Vec2(0, mTitleHeight));
	
	ThePainter->Color.Set(Vec4(0.2, 0.7, 0.9, 1));
	ThePainter->DrawBox(mPosition + mOutputPosition - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	
	ThePainter->Color.Set(Vec4(0.9, 0.9, 0.9, 1));
	float centerX = floor((mSize.x - float(mTitleTexture->TextSize.width())) * 0.5f);
	ThePainter->DrawTextTexture(mTitleTexture, mPosition + Vec2(centerX, TitlePadding + 1));

	for (int i=0; i<mWidgetSlots.size(); i++)
	{
		Vec4 slotFrameColor(1, 1, 1, 0.1);
		if (Panel->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
			if (Panel->mClickedWidget == this && Panel->mClickedSlotIndex == i) {
				slotFrameColor = ConnectionColor;
			}
		} else if (Panel->mHoveredWidget == this && Panel->mHoveredSlotIndex == i) {
			if (Panel->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT) {
				slotFrameColor = Panel->mIsConnectionValid 
					? ConnectionColorValid : ConnectionColorInvalid;
			} else slotFrameColor = Vec4(1, 1, 1, 0.2);
		}

		WidgetSlot* sw = mWidgetSlots[i];
		ThePainter->Color.Set(slotFrameColor);
		//ThePainter->DrawRect(Position.X+2, slotY, Position.X + Size.X - 40, slotY + sw->Text.TextSize.height());
		ThePainter->DrawRect(mPosition + sw->mPosition, sw->mSize);

		ThePainter->Color.Set(Vec4(0.9, 0.9, 0.9, 1));
		ThePainter->DrawTextTexture(&sw->mTexture, mPosition + sw->mPosition + SlotPadding);

		ThePainter->Color.Set(Vec4(0.2, 0.7, 0.9, 1));
		ThePainter->DrawBox(mPosition + sw->mSpotPos - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	}
	
	Vec4 frameColor(1, 1, 1, 0.1);
	if (mIsSelected) {
		frameColor = Vec4(1, 1, 1, 1);
	} else if (Panel->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT 
		&& Panel->mClickedWidget == this) {
			frameColor = ConnectionColor;
	} else if (Panel->mHoveredWidget == this) {
		if (Panel->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
			if (Panel->mClickedWidget != this) {
				frameColor = Panel->mIsConnectionValid ? ConnectionColorValid : ConnectionColorInvalid;
			}
		} else frameColor = Vec4(1, 1, 1, 0.3);
	} 
	ThePainter->Color.Set(frameColor);
	ThePainter->DrawRect(mPosition, mSize);
}

void NodeWidget::HandleTitleChange()
{
  static const QString stubLabel(" [stub]");

  Node* node = GetNode();
  QString text;
  if (!node->GetName().empty()) {
    /// Node has a name, use that.
    text = QString::fromStdString(node->GetName());
  } else {
    /// Just use the type as a name by default
    text = ThePrototypes->GetNodeClassString(GetNode());
    if (node->GetType() == NodeType::SHADER_STUB) {
      ShaderStub* stub = static_cast<ShaderStub*>(node);
      ShaderStubMetadata* metaData = stub->GetStubMetadata();
      if (metaData != nullptr && !metaData->name.empty()) {
        /// For shader stubs, use the stub name by default
        text = QString::fromStdString(metaData->name);
      }
    }
  } 
	mTitleTexture = new TextTexture();
	mTitleTexture->SetText(text, ThePainter->TitleFont);
	OnRepaint();
}

NodeWidget::~NodeWidget()
{
	SafeDelete(mTitleTexture);
}

void NodeWidget::SetPosition( Vec2 Position )
{
	this->mPosition = Position;
	OnRepaint();
}

Vec2 NodeWidget::GetOutputPosition()
{
	return mPosition + mOutputPosition;
}

Vec2 NodeWidget::GetInputPosition( int SlotIndex )
{
	WidgetSlot* sw = mWidgetSlots[SlotIndex];
	return mPosition + sw->mSpotPos;
}

void NodeWidget::HandleSniffedMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_STRUCTURE_CHANGED:
		CreateWidgetSlots();
		break;
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		/// TODO: redraw using this
		break;
  case NodeMessage::NODE_NAME_CHANGED:
    HandleTitleChange();
    break;
	default: break;
	}
}
