#include "uipainter.h"
#include "util.h"
#include <zengine.h>
#include <QTextStream>
#include <QDir>

UiPainter* ThePainter = nullptr;

void InitPainter() {
  ThePainter = new UiPainter();
}

void DisposePainter() {
  SafeDelete(ThePainter);
}

void ConnectToStubParameter(const std::shared_ptr<Pass>& pass, bool vertexShader,
  const char* parameterName, const std::shared_ptr<Node>& target) {
  if (pass == nullptr) {
    ERR("Missing pass.");
    return;
  }
  StubSlot* slot = vertexShader ? &pass->mVertexStub : &pass->mFragmentStub;
  std::shared_ptr<StubNode> stub = slot->GetNode();
  if (!stub) {
    ERR("ShaderStub not connected to pass.");
    return;
  }
  stub->Update();
  Slot* stubSlot = stub->GetSlotByParameterName(std::string(parameterName));
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
  std::shared_ptr<Pass> pass = Util::LoadShader(":/transformPos.vs", ":/solidColor.fs");
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
  mLineMeshNode = std::make_shared<StaticMeshNode>();
  mLineMeshNode->Set(std::make_shared<Mesh>());
  mRectMeshNode = std::make_shared<StaticMeshNode>();
  mRectMeshNode->Set(std::make_shared<Mesh>());

  std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>();
  boxMesh->SetIndices(boxIndices);
  mBoxMeshNode = std::make_shared<StaticMeshNode>();
  mBoxMeshNode->Set(boxMesh);

  std::shared_ptr<Mesh> textureMesh = std::make_shared<Mesh>();
  textureMesh->SetIndices(boxIndices);
  mTexturedBoxMeshNode = std::make_shared<StaticMeshNode>();
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

void UiPainter::DrawRect(const Vec2& topLeft, const Vec2& size) {
  const Vec3 pos(topLeft.x + 0.5f, topLeft.y + 0.5f, 0);
  VertexPos vertices[] = {
    {pos},
    {pos + Vec3(size.x - 1, 0, 0)},
    {pos + Vec3(size.x - 1, size.y - 1, 0)},
    {pos + Vec3(0, size.y - 1, 0)},
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

void UiPainter::DrawTexture(const std::shared_ptr<Texture>& texture, float x, float y) {
  const float w(texture->mWidth);
  const float h(texture->mHeight);
  VertexPosUv vertices[] = {
    {Vec3(x, y, 0), Vec2(0, 0)},
    {Vec3(x + w, y, 0), Vec2(1, 0)},
    {Vec3(x, y + h, 0), Vec2(0, 1)},
    {Vec3(x + w, y + h, 0), Vec2(1, 1)},
  };

  mTextureNode->Set(texture);
  mTexturedBoxMeshNode->GetMesh()->SetVertices(vertices);
  mTexturedBox->Draw(&mGlobals, PassType::SOLID);
}

void UiPainter::SetupViewport(int canvasWidth, int canvasHeight, Vec2 topLeft,
  Vec2 size) {
  OpenGLAPI::SetViewport(0, 0, canvasWidth, canvasHeight);
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
