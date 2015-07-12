#pragma once

#include "textTexture.h"
#include <zengine.h>

class QString;
class UiPainter;

extern UiPainter* ThePainter;

#define ADJUST(x) (x)

void InitPainter();
void DisposePainter();

class UiPainter {
public:
  UiPainter();
  ~UiPainter();

  void Set(int width, int height);

  void DrawLine(float x1, float y1, float x2, float y2);
  void DrawLine(const Vec2& from, const Vec2& to);
  void DrawRect(const Vec2& topLeft, const Vec2& size);
  void DrawBox(const Vec2& topLeft, const Vec2& size);
  void DrawTexture(Texture* texture, float x, float y);
  void DrawTextTexture(TextTexture* texture, const Vec2& position);

  Vec4Node mColor;
  TextureNode	mTextureNode;

  QFont mTitleFont;

private:
  Material mSolidColorMaterial;
  Material mSolidTextureMaterial;
  Material mTextTextureMaterial;

  Texture* mSomeTexture;

  StaticMeshNode* mLineMeshNode;
  StaticMeshNode* mRectMeshNode;
  StaticMeshNode* mBoxMeshNode;
  StaticMeshNode* mTexturedBoxMeshNode;

  Drawable* mSolidLine;
  Drawable* mSolidRect;
  Drawable* mSolidBox;
  Drawable* mTexturedBox;
  Drawable* mTextBox;

  RenderState mCanvasRenderstate;
  Globals mGlobals;
};

