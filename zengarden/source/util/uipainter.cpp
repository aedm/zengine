#include "uipainter.h"
#include "util.h"
#include <zengine.h>
#include <QtCore/QDir>
#include <glm/gtc/matrix_transform.hpp>

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
    {vec3(x1 + 0.5f, y1 + 0.5f, 0)},
    {vec3(x2 + 0.5f, y2 + 0.5f, 0)}
  };
  mLineMeshNode->GetMesh()->SetVertices(vertices);
  mSolidLine->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINES);
}

void UiPainter::DrawLine(const vec2& From, const vec2& To) {
  VertexPos vertices[] = {
    {vec3(From.x + 0.5f, From.y + 0.5f, 0)},
    {vec3(To.x + 0.5f, To.y + 0.5f, 0)} };
  mLineMeshNode->GetMesh()->SetVertices(vertices);
  mSolidLine->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINES);
}

void UiPainter::DrawRect(const vec2& topLeft, const vec2& size) {
  const vec3 pos(topLeft.x + 0.5f, topLeft.y + 0.5f, 0);
  VertexPos vertices[] = {
    {pos},
    {pos + vec3(size.x - 1, 0, 0)},
    {pos + vec3(size.x - 1, size.y - 1, 0)},
    {pos + vec3(0, size.y - 1, 0)},
    {pos},
  };
  mRectMeshNode->GetMesh()->SetVertices(vertices);
  mSolidRect->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_LINE_STRIP);
}


void UiPainter::DrawBox(const vec2& TopLeft, const vec2& Size) {
  VertexPos vertices[] = {
    {vec3(TopLeft.x, TopLeft.y, 0)},
    {vec3(TopLeft.x + Size.x, TopLeft.y, 0)},
    {vec3(TopLeft.x, TopLeft.y + Size.y, 0)},
    {vec3(TopLeft.x + Size.x, TopLeft.y + Size.y, 0)},
  };
  mBoxMeshNode->GetMesh()->SetVertices(vertices);
  mSolidBox->Draw(&mGlobals, PassType::SOLID, PRIMITIVE_TRIANGLES);
}

void UiPainter::DrawTexture(const std::shared_ptr<Texture>& texture, float x, float y) {
  const float w(texture->mWidth);
  const float h(texture->mHeight);
  VertexPosUv vertices[] = {
    {vec3(x, y, 0), vec2(0, 0)},
    {vec3(x + w, y, 0), vec2(1, 0)},
    {vec3(x, y + h, 0), vec2(0, 1)},
    {vec3(x + w, y + h, 0), vec2(1, 1)},
  };

  mTextureNode->Set(texture);
  mTexturedBoxMeshNode->GetMesh()->SetVertices(vertices);
  mTexturedBox->Draw(&mGlobals, PassType::SOLID);
}

void UiPainter::SetupViewport(int canvasWidth, int canvasHeight, vec2 topLeft,
  vec2 size) {
  OpenGLAPI::SetViewport(0, 0, canvasWidth, canvasHeight);
  mColor->Set(vec4(1, 1, 1, 1));

  mGlobals.RenderTargetSize = vec2(canvasWidth, canvasHeight);
  mGlobals.RenderTargetSizeRecip =
    vec2(1.0f / float(canvasWidth), 1.0f / float(canvasHeight));

  mGlobals.Camera = mat4(1.0f);
  mGlobals.World = mat4(1.0f);
  mGlobals.Projection =
    glm::ortho(topLeft.x, topLeft.x + size.x, topLeft.y + size.y, topLeft.y);
  mGlobals.Transformation = mGlobals.Projection;
}
