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

  void SetupViewport(int canvasWidth, int canvasHeight, vec2 topLeft, vec2 size);

  void DrawLine(float x1, float y1, float x2, float y2);
  void DrawLine(const vec2& from, const vec2& to);
  void DrawRect(const vec2& topLeft, const vec2& size);
  void DrawBox(const vec2& topLeft, const vec2& size);
  void DrawTexture(const std::shared_ptr<Texture>& texture, float x, float y);

  std::shared_ptr<Vec4Node> mColor = std::make_shared<Vec4Node>();
  std::shared_ptr<StaticTextureNode> mTextureNode = std::make_shared<StaticTextureNode>();

  QFont mTitleFont;

private:
  const std::shared_ptr<Material> mSolidColorMaterial = std::make_shared<Material>();
  const std::shared_ptr<Material> mSolidTextureMaterial = std::make_shared<Material>();
  const std::shared_ptr<Material> mTextTextureMaterial = std::make_shared<Material>();

  std::shared_ptr<StaticMeshNode> mLineMeshNode;
  std::shared_ptr<StaticMeshNode> mRectMeshNode;
  std::shared_ptr<StaticMeshNode> mBoxMeshNode;
  std::shared_ptr<StaticMeshNode> mTexturedBoxMeshNode;

  const std::shared_ptr<Drawable> mSolidLine = std::make_shared<Drawable>();
  const std::shared_ptr<Drawable> mSolidRect = std::make_shared<Drawable>();
  const std::shared_ptr<Drawable> mSolidBox = std::make_shared<Drawable>();
  const std::shared_ptr<Drawable> mTexturedBox = std::make_shared<Drawable>();
  const std::shared_ptr<Drawable> mTextBox = std::make_shared<Drawable>();

  Globals mGlobals{};
};

