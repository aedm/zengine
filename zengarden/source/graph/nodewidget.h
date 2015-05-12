#pragma once

#include "../util/textTexture.h"
#include "grapheditor.h"
#include "../watchers/watcher.h"
#include <zengine.h>

class NodeWidget: public Watcher {
	friend class MoveNodeCommand;
	friend class GraphEditor;

public:
	NodeWidget(Node* Nd);
	~NodeWidget();

	void				Paint(GraphEditor* Panel);
	void				SetTitle(const QString& Title);
	Vec2				GetOutputPosition();
	Vec2				GetInputPosition(int SlotIndex);

	Event<>				EventRepaint;

private:
	virtual void		HandleSniffedMessage(Slot* S, NodeMessage Message, 
							const void* Payload) override;

	/// Command-accessible
	void				SetPosition(Vec2 Position);

	/// Layout
	void				CalculateLayout();

	Vec2				Position;
	Vec2				Size;
	Vec2				OutputPosition;

	/// Viewer states
	bool				Selected;
	Vec2				OriginalPosition;
	Vec2				OriginalSize;

	/// Height of the titlebar
	float				TitleHeight;

	struct WidgetSlot
	{
		TextTexture		Text;
		Vec2			Position;
		Vec2			Size;
		Vec2			SpotPos;
		Slot*			TheSlot;
	};

	vector<WidgetSlot*>	WidgetSlots;
	void				CreateWidgetSlots();

	TextTexture*		TitleTexture;
};

