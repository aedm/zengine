#pragma once

#include "../util/textTexture.h"
#include "operatorpanel.h"
#include <zengine.h>

class OperatorWidget {
	friend class MoveOperatorCommand;
	friend class OperatorPanel;

public:
	OperatorWidget(Node* Nd);
	~OperatorWidget();

	void				Paint(OperatorPanel* Panel);

	void				SetTitle(const QString& Title);

	Vec2				GetOutputPosition();
	Vec2				GetInputPosition(int SlotIndex);

	Node*				GetOperator();

	Event<void>			EventRepaint;

private:
	/// Command-accessible
	void				SetPosition(Vec2 Position);

	Node*				Op;

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

	struct SlotWidget
	{
		TextTexture		Text;
		Vec2			Position;
		Vec2			Size;
		Vec2			SpotPos;
	};

	vector<SlotWidget*>	Slots;

	TextTexture*		TitleTexture;
};

