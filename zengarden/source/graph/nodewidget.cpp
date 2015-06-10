#include "nodewidget.h"
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


NodeWidget::NodeWidget(Node* Nd)
	: Watcher(Nd, nullptr, NodeType::WIDGET)
	, TitleTexture(NULL)
{
	Selected = false;
	SetTitle(QString::fromStdString(Nd->mName));

	CreateWidgetSlots();
}


void NodeWidget::CreateWidgetSlots()
{
	for (auto x : WidgetSlots) delete x;
	WidgetSlots.clear();

	Node* node = GetNode();
	for(Slot* slot : node->mSlots)
	{
		WidgetSlot* sw = new WidgetSlot();
		sw->Text.SetText(QString::fromStdString(*slot->GetName()), ThePainter->TitleFont);
		sw->TheSlot = slot;
		WidgetSlots.push_back(sw);
	}
	CalculateLayout();
}


void NodeWidget::CalculateLayout()
{
	TitleHeight = TitleTexture->TextSize.height() + TitlePadding * 2.0f + 1.0f;
	float slotY = TitleHeight + SlotSpacing;
	for(WidgetSlot* sw : WidgetSlots)
	{
		sw->Position = Vec2(SlotLeftMargin, slotY);
		sw->Size = Vec2(SlotWidth, float(sw->Text.TextSize.height()) + SlotPadding.y * 2.0f);
		sw->SpotPos = Vec2(ConnectionSpotPadding, slotY + sw->Size.y / 2.0f);
		slotY += sw->Size.y + SlotSpacing;
	}
	Size = Vec2(SlotLeftMargin + SlotWidth + SlotRightMargin, slotY + 1);
	OutputPosition = Vec2(Size.x - ConnectionSpotPadding - 1.0f, TitleHeight / 2.0f);
}


void NodeWidget::Paint(GraphEditor* Panel)
{
	ThePainter->Color.Set(Vec4(0, 0.2, 0.4, Opacity));
	ThePainter->DrawBox(Position, Vec2(Size.x, TitleHeight));

	ThePainter->Color.Set(Vec4(0, 0, 0, Opacity));
	ThePainter->DrawBox(Position + Vec2(0, TitleHeight), Size - Vec2(0, TitleHeight));
	
	ThePainter->Color.Set(Vec4(0.2, 0.7, 0.9, 1));
	ThePainter->DrawBox(Position + OutputPosition - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	
	ThePainter->Color.Set(Vec4(0.9, 0.9, 0.9, 1));
	float centerX = floor((Size.x - float(TitleTexture->TextSize.width())) * 0.5f);
	ThePainter->DrawTextTexture(TitleTexture, Position + Vec2(centerX, TitlePadding + 1));

	for (int i=0; i<WidgetSlots.size(); i++)
	{
		Vec4 slotFrameColor(1, 1, 1, 0.1);
		if (Panel->CurrentState == GraphEditor::State::CONNECT_TO_NODE) {
			if (Panel->ClickedWidget == this && Panel->ClickedSlot == i) {
				slotFrameColor = ConnectionColor;
			}
		} else if (Panel->HoveredWidget == this && Panel->HoveredSlot == i) {
			if (Panel->CurrentState == GraphEditor::State::CONNECT_TO_SLOT) {
				slotFrameColor = Panel->ConnectionValid 
					? ConnectionColorValid : ConnectionColorInvalid;
			} else slotFrameColor = Vec4(1, 1, 1, 0.2);
		}

		WidgetSlot* sw = WidgetSlots[i];
		ThePainter->Color.Set(slotFrameColor);
		//ThePainter->DrawRect(Position.X+2, slotY, Position.X + Size.X - 40, slotY + sw->Text.TextSize.height());
		ThePainter->DrawRect(Position + sw->Position, sw->Size);

		ThePainter->Color.Set(Vec4(0.9, 0.9, 0.9, 1));
		ThePainter->DrawTextTexture(&sw->Text, Position + sw->Position + SlotPadding);

		ThePainter->Color.Set(Vec4(0.2, 0.7, 0.9, 1));
		ThePainter->DrawBox(Position + sw->SpotPos - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	}
	
	Vec4 frameColor(1, 1, 1, 0.1);
	if (Selected) {
		frameColor = Vec4(1, 1, 1, 1);
	} else if (Panel->CurrentState == GraphEditor::State::CONNECT_TO_SLOT 
		&& Panel->ClickedWidget == this) {
			frameColor = ConnectionColor;
	} else if (Panel->HoveredWidget == this) {
		if (Panel->CurrentState == GraphEditor::State::CONNECT_TO_NODE) {
			if (Panel->ClickedWidget != this) {
				frameColor = Panel->ConnectionValid ? ConnectionColorValid : ConnectionColorInvalid;
			}
		} else frameColor = Vec4(1, 1, 1, 0.3);
	} 
	ThePainter->Color.Set(frameColor);
	ThePainter->DrawRect(Position, Size);
}

void NodeWidget::SetTitle( const QString& Title )
{
	TitleTexture = new TextTexture();
	TitleTexture->SetText(Title, ThePainter->TitleFont);
	EventRepaint();
}

NodeWidget::~NodeWidget()
{
	SafeDelete(TitleTexture);
}

void NodeWidget::SetPosition( Vec2 Position )
{
	this->Position = Position;
	EventRepaint();
}

Vec2 NodeWidget::GetOutputPosition()
{
	return Position + OutputPosition;
}

Vec2 NodeWidget::GetInputPosition( int SlotIndex )
{
	WidgetSlot* sw = WidgetSlots[SlotIndex];
	return Position + sw->SpotPos;
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
	default: break;
	}
}
