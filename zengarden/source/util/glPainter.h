#pragma once

#include "textTexture.h"
#include <zengine.h>

class QString;
class GLPainter;

extern GLPainter* ThePainter;

ShaderOperator*	LoadShader(const char* VertexFile, const char* FragmentFile);
OWNERSHIP char*	ReadFileQt(const char* FileName);
#define ADJUST(x) (x)

void InitCanvas();
void DisposeCanvas();

class GLPainter 
{
public:
	GLPainter();
	~GLPainter();

	void					Set(int Width, int Height);

	void					DrawLine(float x1, float y1, float x2, float y2);
	void					DrawLine(const Vec2& From, const Vec2& To);
	void					DrawRect(const Vec2& TopLeft, const Vec2& Size);
	void					DrawBox(const Vec2& TopLeft, const Vec2& Size);
	void					DrawTexture(Texture* Tex, float x, float y);
	void					DrawTextTexture(TextTexture* Tex, const Vec2& Position);

	Vec4Operator			Color;
	TextureOperator			TexOp;

	QFont					TitleFont;

private:
	ShaderOperator*			SolidColorOp;
	ShaderOperator*			SolidTextureOp;
	ShaderOperator*			TextTextureOp;

	Texture*				SomeTexture;

	MeshOperator*			LineMeshOp;
	MeshOperator*			RectMeshOp;
	MeshOperator*			BoxMeshOp;
	MeshOperator*			TexturedBoxMeshOp;

	Model*					SolidLineModel;
	Model*					SolidRectModel;
	Model*					SolidBoxModel;
	Model*					TexturedBoxModel;
	Model*					TextBoxModel;

	RenderState				CanvasRenderstate;
			
};

