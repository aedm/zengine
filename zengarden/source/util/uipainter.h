#pragma once

#include <zengine.h>
#include <QFont>

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

  void SetupViewport(int canvasWidth, int canvasHeight, Vec2 topLeft, Vec2 size);

  void DrawLine(float x1, float y1, float x2, float y2);
  void DrawLine(const Vec2& from, const Vec2& to);
  void DrawRect(const Vec2& topLeft, const Vec2& size);
  void DrawBox(const Vec2& topLeft, const Vec2& size);
  void DrawTexture(const shared_ptr<Texture>& texture, float x, float y);

  shared_ptr<Vec4Node> mColor = make_shared<Vec4Node>();
  shared_ptr<StaticTextureNode> mTextureNode = make_shared<StaticTextureNode>();

  QFont mTitleFont;

private:
  const shared_ptr<Material> mSolidColorMaterial = make_shared<Material>();
  const shared_ptr<Material> mSolidTextureMaterial = make_shared<Material>();
  const shared_ptr<Material> mTextTextureMaterial = make_shared<Material>();

  shared_ptr<StaticMeshNode> mLineMeshNode;
  shared_ptr<StaticMeshNode> mRectMeshNode;
  shared_ptr<StaticMeshNode> mBoxMeshNode;
  shared_ptr<StaticMeshNode> mTexturedBoxMeshNode;

  const shared_ptr<Drawable> mSolidLine = make_shared<Drawable>();
  const shared_ptr<Drawable> mSolidRect = make_shared<Drawable>();
  const shared_ptr<Drawable> mSolidBox = make_shared<Drawable>();
  const shared_ptr<Drawable> mTexturedBox = make_shared<Drawable>();
  const shared_ptr<Drawable> mTextBox = make_shared<Drawable>();

  Globals mGlobals{};
};

