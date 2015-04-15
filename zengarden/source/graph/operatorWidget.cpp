#include "operatorWidget.h"
#include "../util/glPainter.h"
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


OperatorWidget::OperatorWidget(Node* _Nd)
	: TitleTexture(NULL)
	, Op(_Nd)
{
	Selected = false;
	SetTitle(QString::fromStdString(Op->Name));

	foreach(Slot* slot, Op->Slots)
	{
		SlotWidget* sw = new SlotWidget();
		sw->Text.SetText(QString::fromStdString(*slot->GetName()), ThePainter->TitleFont);
		Slots.push_back(sw);
	}

	CalculateLayout();
}


void OperatorWidget::CalculateLayout()
{
	TitleHeight = TitleTexture->TextSize.height() + TitlePadding * 2.0f + 1.0f;
	float slotY = TitleHeight + SlotSpacing;
	foreach(SlotWidget* sw, Slots)
	{
		sw->Position = Vec2(SlotLeftMargin, slotY);
		sw->Size = Vec2(SlotWidth, float(sw->Text.TextSize.height()) + SlotPadding.Y * 2.0f);
		sw->SpotPos = Vec2(ConnectionSpotPadding, slotY + sw->Size.Y / 2.0f);
		slotY += sw->Size.Y + SlotSpacing;
	}
	Size = Vec2(SlotLeftMargin + SlotWidth + SlotRightMargin, slotY + 1);
	OutputPosition = Vec2(Size.X - ConnectionSpotPadding - 1.0f, TitleHeight / 2.0f);
}


void OperatorWidget::Paint(OperatorPanel* Panel)
{
	ThePainter->Color.SetValue(Vec4(0, 0.2, 0.4, Opacity));
	ThePainter->DrawBox(Position, Vec2(Size.X, TitleHeight));

	ThePainter->Color.SetValue(Vec4(0, 0, 0, Opacity));
	ThePainter->DrawBox(Position + Vec2(0, TitleHeight), Size - Vec2(0, TitleHeight));
	
	ThePainter->Color.SetValue(Vec4(0.2, 0.7, 0.9, 1));
	ThePainter->DrawBox(Position + OutputPosition - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	
	ThePainter->Color.SetValue(Vec4(0.9, 0.9, 0.9, 1));
	float centerX = floor((Size.X - float(TitleTexture->TextSize.width())) * 0.5f);
	ThePainter->DrawTextTexture(TitleTexture, Position + Vec2(centerX, TitlePadding + 1));

	for (int i=0; i<Slots.size(); i++)
	{
		Vec4 slotFrameColor(1, 1, 1, 0.1);
		if (Panel->State == OperatorPanel::STATE_CONNECTTOOPERATOR) {
			if (Panel->ClickedWidget == this && Panel->ClickedSlot == i) {
				slotFrameColor = ConnectionColor;
			}
		} else if (Panel->HoveredWidget == this && Panel->HoveredSlot == i) {
			if (Panel->State == OperatorPanel::STATE_CONNECTTOSLOT) {
				slotFrameColor = Panel->ConnectionValid 
					? ConnectionColorValid : ConnectionColorInvalid;
			} else slotFrameColor = Vec4(1, 1, 1, 0.6);
		}

		SlotWidget* sw = Slots[i];
		ThePainter->Color.SetValue(slotFrameColor);
		//ThePainter->DrawRect(Position.X+2, slotY, Position.X + Size.X - 40, slotY + sw->Text.TextSize.height());
		ThePainter->DrawRect(Position + sw->Position, sw->Size);

		ThePainter->Color.SetValue(Vec4(0.9, 0.9, 0.9, 1));
		ThePainter->DrawTextTexture(&sw->Text, Position + sw->Position + SlotPadding);

		ThePainter->Color.SetValue(Vec4(0.2, 0.7, 0.9, 1));
		ThePainter->DrawBox(Position + sw->SpotPos - ConnectionSpotSize * 0.5f, ConnectionSpotSize);
	}
	
	Vec4 frameColor(1, 1, 1, 0.1);
	if (Selected) {
		frameColor = Vec4(1, 1, 1, 1);
	} else if (Panel->State == OperatorPanel::STATE_CONNECTTOSLOT 
		&& Panel->ClickedWidget == this) {
			frameColor = ConnectionColor;
	} else if (Panel->HoveredWidget == this) {
		if (Panel->State == OperatorPanel::STATE_CONNECTTOOPERATOR) {
			if (Panel->ClickedWidget != this) {
				frameColor = Panel->ConnectionValid ? ConnectionColorValid : ConnectionColorInvalid;
			}
		} else frameColor = Vec4(1, 1, 1, 0.3);
	} 
	ThePainter->Color.SetValue(frameColor);
	ThePainter->DrawRect(Position, Size);
}

void OperatorWidget::SetTitle( const QString& Title )
{
	TitleTexture = new TextTexture();
	TitleTexture->SetText(Title, ThePainter->TitleFont);
	EventRepaint();
}

OperatorWidget::~OperatorWidget()
{
	SafeDelete(TitleTexture);
}

void OperatorWidget::SetPosition( Vec2 Position )
{
	this->Position = Position;
	EventRepaint();
}

Node* OperatorWidget::GetOperator()
{
	return Op;
}

Vec2 OperatorWidget::GetOutputPosition()
{
	return Position + OutputPosition;
}

Vec2 OperatorWidget::GetInputPosition( int SlotIndex )
{
	SlotWidget* sw = Slots[SlotIndex];
	return Position + sw->SpotPos;
}
