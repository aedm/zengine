#include "nodewidget.h"
#include "prototypes.h"
#include "../util/uipainter.h"
#include "../zengarden.h"
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


NodeWidget::NodeWidget(const shared_ptr<Node>& node, GraphWatcher* graphWatcher)
	: WatcherUI(node)
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

  mGraphWatcher->GetGLWidget()->makeCurrent();
  for (Slot* slot : mNode->GetPublicSlots())
	{
    if (slot->DoesAcceptNode(StaticValueNodesList[int(ValueType::STRING)])) continue;
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


void NodeWidget::UpdateGraph() {
  if (mGraphWatcher) mGraphWatcher->Update();
}

static Vec4 LiveHeaderColor = Vec4(0, 0.2, 0.4, Opacity);
static Vec4 GhostHeaderColor = Vec4(0.4, 0.2, 0, Opacity);

void NodeWidget::Paint()
{
  Vec2 position = mNode->GetPosition();
  Vec2 size = mNode->GetSize();

  ThePainter->mColor->Set(mNode->IsGhostNode() ? GhostHeaderColor : LiveHeaderColor);
	ThePainter->DrawBox(position, Vec2(size.x, mTitleHeight));

	ThePainter->mColor->Set(Vec4(0, 0, 0, Opacity));
	ThePainter->DrawBox(position + Vec2(0, mTitleHeight), size - Vec2(0, mTitleHeight));
	
	ThePainter->mColor->Set(Vec4(0.2, 0.7, 0.9, 1));
	ThePainter->DrawBox(position + mOutputPosition - ConnectionSpotSize * 0.5f, 
                      ConnectionSpotSize);
	
	ThePainter->mColor->Set(Vec4(0.9, 0.9, 0.9, 1));
	float centerX = floor((size.x - float(mTitleTexture->TextSize.width())) * 0.5f);
	ThePainter->DrawTextTexture(mTitleTexture, position + Vec2(centerX, TitlePadding + 1));

  /// Paint slots
	for (int i=0; i<mWidgetSlots.size(); i++)
	{
		Vec4 slotFrameColor(1, 1, 1, 0.1);
		if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
      if (mGraphWatcher->mClickedWidget.get() == this 
          && mGraphWatcher->mClickedSlotIndex == i) {
				slotFrameColor = ConnectionColor;
			}
    } else if (mGraphWatcher->mHoveredWidget.get() == this 
               && mGraphWatcher->mHoveredSlotIndex == i) {
      if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT) {
        slotFrameColor = mGraphWatcher->mIsConnectionValid
					? ConnectionColorValid : ConnectionColorInvalid;
			} else slotFrameColor = Vec4(1, 1, 1, 0.2);
		}

		WidgetSlot* sw = mWidgetSlots[i];
		ThePainter->mColor->Set(slotFrameColor);
		ThePainter->DrawRect(position + sw->mPosition, sw->mSize);

		ThePainter->mColor->Set(Vec4(0.9, 0.9, 0.9, 1));
		ThePainter->DrawTextTexture(&sw->mTexture, position + sw->mPosition + SlotPadding);

		ThePainter->mColor->Set(Vec4(0.2, 0.7, 0.9, 1));
		ThePainter->DrawBox(position + sw->mSpotPos - ConnectionSpotSize * 0.5f, 
                        ConnectionSpotSize);
	}
	
  /// Paint frame
	Vec4 frameColor(1, 1, 1, 0.1);
	if (mIsSelected) {
		frameColor = Vec4(1, 1, 1, 1);
  } else if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_SLOT
             && mGraphWatcher->mClickedWidget.get() == this) {
			frameColor = ConnectionColor;
  } else if (mGraphWatcher->mHoveredWidget.get() == this) {
    if (mGraphWatcher->mCurrentState == GraphWatcher::State::CONNECT_TO_NODE) {
      if (mGraphWatcher->mClickedWidget.get() != this) {
        frameColor = mGraphWatcher->mIsConnectionValid
          ? ConnectionColorValid : ConnectionColorInvalid;
			}
		} else frameColor = Vec4(1, 1, 1, 0.3);
	} 
	ThePainter->mColor->Set(frameColor);
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
  UpdateGraph();
  shared_ptr<Node> node = ZenGarden::GetInstance()->GetNodeInPropertyEditor();
  if (node == mNode) {
    ZenGarden::GetInstance()->SetNodeForPropertyEditor(node);;
  }
}


void NodeWidget::OnNameChange() {
  static const QString stubLabel(" [stub]");

  QString text;
  if (!mNode->GetName().empty()) {
    /// Node has a name, use that.
    text = QString::fromStdString(mNode->GetName());
  } else {
    /// Just use the type as a name by default
    shared_ptr<Node> node = mNode->GetReferencedNode();
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
  mTitleTexture = new TextTexture();
  mTitleTexture->SetText(text, ThePainter->mTitleFont);
  UpdateGraph();
}


void NodeWidget::OnGraphPositionChanged() {
  UpdateGraph();
}


void NodeWidget::OnSlotConnectionChanged(Slot* slot) {
  UpdateGraph();
}
