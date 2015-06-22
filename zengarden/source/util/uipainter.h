#pragma once

#include "textTexture.h"
#include <zengine.h>

class QString;
class UiPainter;

extern UiPainter* ThePainter;

#define ADJUST(x) (x)

void InitPainter();
void DisposePainter();

class UiPainter 
{
public:
	UiPainter();
	~UiPainter();

	void					Set(int Width, int Height);

	void					DrawLine(float x1, float y1, float x2, float y2);
	void					DrawLine(const Vec2& From, const Vec2& To);
	void					DrawRect(const Vec2& TopLeft, const Vec2& Size);
	void					DrawBox(const Vec2& TopLeft, const Vec2& Size);
	void					DrawTexture(Texture* Tex, float x, float y);
	void					DrawTextTexture(TextTexture* Tex, const Vec2& Position);

	Vec4Node				Color;
	TextureNode				TexOp;

	QFont					TitleFont;

private:
	Material SolidColorOp;
  Material SolidTextureOp;
  Material TextTextureOp;

	Texture*				SomeTexture;

	StaticMeshNode*			LineMeshOp;
	StaticMeshNode*			RectMeshOp;
	StaticMeshNode*			BoxMeshOp;
	StaticMeshNode*			TexturedBoxMeshOp;

	Drawable*			SolidLineModel;
  Drawable*			SolidRectModel;
  Drawable*			SolidBoxModel;
  Drawable*			TexturedBoxModel;
  Drawable*			TextBoxModel;

	RenderState				CanvasRenderstate;
  Globals mGlobals;
};

