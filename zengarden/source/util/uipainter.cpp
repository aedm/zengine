#include "uipainter.h"
#include "util.h"
#include <zengine.h>
#include <QTextStream>
#include <QDir>
#include <QString>
#include <QByteArray>

UiPainter* ThePainter = nullptr;

void InitPainter() {
  ThePainter = new UiPainter();
}

void DisposePainter() {
  SafeDelete(ThePainter);
}

void ConnectToStubParameter(const shared_ptr<Pass>& pass, bool vertexShader,
  const char* parameterName, const shared_ptr<Node>& target) {
  if (pass == nullptr) {
    ERR("Missing pass.");
    return;
  }
  StubSlot* slot = vertexShader ? &pass->mVertexStub : &pass->mFragmentStub;
  shared_ptr<StubNode> stub = slot->GetNode();
  if (!stub) {
    ERR("ShaderStub not connected to pass.");
    return;
  }
  stub->Update();
  Slot* stubSlot = stub->GetSlotByParameterName(string(parameterName));
  if (!stubSlot) {
    ERR("No shader parameter called %s.", parameterName);
    return;
  }
  stubSlot->Connect(target);
}

UiPainter::UiPainter() {
  /// Fonts
  mTitleFont.setPixelSize(ADJUST(11));

  /// Shaders
  shared_ptr<Pass> pass = Util::LoadShader(":/transformPos.vs", ":/solidColor.fs");
  ConnectToStubParameter(pass, false, "uColor", mColor);
  pass->mRenderstate.mDepthTest = false;
  mSolidColorMaterial->mSolidPass.Connect(pass);

  pass = Util::LoadShader(":/transformPosUV.vs", ":/solidTexture.fs");
  ConnectToStubParameter(pass, false, "uTexture", mTextureNode);
  pass->mRenderstate.mDepthTest = false;
  mSolidTextureMaterial->mSolidPass.Connect(pass);

  pass = Util::LoadShader(":/transformPosUV.vs", ":/textTexture.fs");
  ConnectToStubParameter(pass, false, "uColor", mColor);
  ConnectToStubParameter(pass, false, "uTexture", mTextureNode);
  pass->mRenderstate.mDepthTest = false;
  mTextTextureMaterial->mSolidPass.Connect(pass);

  IndexEntry boxIndices[] = { 0, 1, 2, 2, 1, 3 };

  /// Meshes
  mLineMeshNode = make_shared<StaticMeshNode>();
  mLineMeshNode->Set(make_shared<Mesh>());
  mRectMeshNode = make_shared<StaticMeshNode>();
  mRectMeshNode->Set(make_shared<Mesh>());

  shared_ptr<Mesh> boxMesh = make_shared<Mesh>();
  boxMesh->SetIndices(boxIndices);
  mBoxMeshNode = make_shared<StaticMeshNode>();
  mBoxMeshNode->Set(boxMesh);

  shared_ptr<Mesh> textureMesh = make_shared<Mesh>();
  textureMesh->SetIndices(boxIndices);
  mTexturedBoxMeshNode = make_shared<StaticMeshNode>();
  mTexturedBoxMeshNode->Set(textureMesh);

  /// Models
  mSolidLine->mMaterial.Connect(mSolidColorMaterial);
  mSolidLine->mMesh.Connect(mLineMeshNode);

  mSolidRect->mMaterial.Connect(mSolidColorMaterial);
  mSolidRect->mMesh.Connect(mRectMeshNode);

  mSolidBox->mMaterial.Connect(mSolidColorMaterial);
  mSolidBox->mMesh.Connect(mBoxMeshNode);

  mTexturedBox->mMaterial.Connect(mSolidTextureMaterial);
  mTexturedBox->mMesh.Connect(mTexturedBoxMeshNode);

  mTextBox->mMaterial.Connect(mTextTextureMaterial);
  mTextBox->mMesh.Connect(mTexturedBoxMeshNode);
}

UiPainter::~UiPainter() = default;

void UiPainter::DrawLine(float x1, float y1, float x2, float y2) {
  VertexPos vertices[] = {
    {Vec3(x1 + 0.5f, y1 + 0.5f, 0)},
    {Vec3(x2 + 0.5f, y2 + 0.5f, 0)}
  };
  mLineMeshNode->GetMesh()->SetVertices(vertices);
  mSolidLine->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINES);
}

void UiPainter::DrawLine(const Vec2& From, const Vec2& To) {
  VertexPos vertices[] = {
    {Vec3(From.x + 0.5f, From.y + 0.5f, 0)},
    {Vec3(To.x + 0.5f, To.y + 0.5f, 0)} };
  mLineMeshNode->GetMesh()->SetVertices(vertices);
  mSolidLine->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINES);
}

void UiPainter::DrawRect(const Vec2& TopLeft, const Vec2& Size) {
  const Vec3 pos(TopLeft.x + 0.5f, TopLeft.y + 0.5f, 0);
  VertexPos vertices[] = {
    {pos},
    {pos + Vec3(Size.x - 1, 0, 0)},
    {pos + Vec3(Size.x - 1, Size.y - 1, 0)},
    {pos + Vec3(0, Size.y - 1, 0)},
    {pos},
  };
  mRectMeshNode->GetMesh()->SetVertices(vertices);
  mSolidRect->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINE_STRIP);
}


void UiPainter::DrawBox(const Vec2& TopLeft, const Vec2& Size) {
  VertexPos vertices[] = {
    {Vec3(TopLeft.x, TopLeft.y, 0)},
    {Vec3(TopLeft.x + Size.x, TopLeft.y, 0)},
    {Vec3(TopLeft.x, TopLeft.y + Size.y, 0)},
    {Vec3(TopLeft.x + Size.x, TopLeft.y + Size.y, 0)},
  };
  mBoxMeshNode->GetMesh()->SetVertices(vertices);
  mSolidBox->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_TRIANGLES);
}

void UiPainter::DrawTexture(const shared_ptr<Texture>& texture, float x, float y) {
  const float w(texture->mWidth);
  const float h(texture->mHeight);
  VertexPosUV vertices[] = {
    {Vec3(x, y, 0), Vec2(0, 0)},
    {Vec3(x + w, y, 0), Vec2(1, 0)},
    {Vec3(x, y + h, 0), Vec2(0, 1)},
    {Vec3(x + w, y + h, 0), Vec2(1, 1)},
  };

  mTextureNode->Set(texture);
  mTexturedBoxMeshNode->GetMesh()->SetVertices(vertices);
  mTexturedBox->Draw(&mGlobals, PassType::SOLID);
}


void UiPainter::DrawTextTexture(TextTexture* Tex, const Vec2& Position) {
  const float w = Tex->mTextSize.width();
  const float h = Tex->mTextSize.height();
  const shared_ptr<Texture> texture = Tex->GetTexture();
  const float u = w / float(texture->mWidth);
  const float v = h / float(texture->mHeight);
  VertexPosUV vertices[] = {
    {Vec3(Position.x, Position.y, 0), Vec2(0, 0)},
    {Vec3(Position.x + w, Position.y, 0), Vec2(u, 0)},
    {Vec3(Position.x, Position.y + h, 0), Vec2(0, v)},
    {Vec3(Position.x + w, Position.y + h, 0), Vec2(u, v)},
  };

  mTextureNode->Set(texture);
  mTexturedBoxMeshNode->GetMesh()->SetVertices(vertices);
  mTextBox->Draw(&mGlobals, PassType::SOLID);
}


void UiPainter::SetupViewport(int canvasWidth, int canvasHeight, Vec2 topLeft,
  Vec2 size) {
  OpenGL->SetViewport(0, 0, canvasWidth, canvasHeight);
  mColor->Set(Vec4(1, 1, 1, 1));

  mGlobals.RenderTargetSize = Vec2(canvasWidth, canvasHeight);
  mGlobals.RenderTargetSizeRecip =
    Vec2(1.0f / float(canvasWidth), 1.0f / float(canvasHeight));

  mGlobals.Camera.LoadIdentity();
  mGlobals.World.LoadIdentity();
  mGlobals.Projection =
    Matrix::Ortho(topLeft.x, topLeft.y, topLeft.x + size.x, topLeft.y + size.y);
  mGlobals.Transformation = mGlobals.Projection;
}
