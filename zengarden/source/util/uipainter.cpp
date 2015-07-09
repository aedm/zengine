#include "uipainter.h"
#include "util.h"
#include <zengine.h>
#include <QTextStream>
#include <QDir>
#include <QString>
#include <QByteArray>

UiPainter* ThePainter = NULL;

void InitPainter() {
  ThePainter = new UiPainter();
}

void DisposePainter() {
  SafeDelete(ThePainter);
}

void ConnectToStubParameter(Pass* pass, bool vertexShader, 
                            const char* parameterName, Node* target) {
  if (pass == nullptr) {
    ERR("Missing pass.");
    return;
  }
  StubSlot* slot = vertexShader ? &pass->mVertexStub : &pass->mFragmentStub;
  ShaderStub* stub = slot->GetNode();
  if (!stub) {
    ERR("ShaderStub not connected to pass.");
    return;
  }
  Slot* stubSlot = stub->GetSlotByParameterName(string(parameterName));
  if (!stubSlot) {
    ERR("No shader parameter called %s.", parameterName);
    return;
  }
  stubSlot->Connect(target);
}

UiPainter::UiPainter() {
  /// Fonts
  TitleFont.setPixelSize(ADJUST(11));

  /// Shaders
  Pass* pass = Util::LoadShader(":/transformPos.vs", ":/solidColor.fs");
  ConnectToStubParameter(pass, false, "uColor", &Color);
  SolidColorOp.mSolidPass.Connect(pass);
  
  pass = Util::LoadShader(":/transformPosUV.vs", ":/solidTexture.fs");
  ConnectToStubParameter(pass, false, "uTexture", &TexOp);
  SolidTextureOp.mSolidPass.Connect(pass);

  pass = Util::LoadShader(":/transformPosUV.vs", ":/textTexture.fs");
  ConnectToStubParameter(pass, false, "uColor", &Color);
  ConnectToStubParameter(pass, false, "uTexture", &TexOp);
  TextTextureOp.mSolidPass.Connect(pass);

  /// Create a texture
  const int m = 256;
  UINT tex[m*m];
  for (UINT y = 0; y < m; y++) for (UINT x = 0; x < m; x++) tex[y*m + x] = (x^y) | 0xff000000;
  SomeTexture = TheResourceManager->CreateTexture(m, m, TEXELTYPE_RGBA_UINT8, tex);

  IndexEntry boxIndices[] = {0, 1, 2, 2, 1, 3};

  /// Meshes
  LineMeshOp = StaticMeshNode::Create(TheResourceManager->CreateMesh());
  RectMeshOp = StaticMeshNode::Create(TheResourceManager->CreateMesh());

  Mesh* boxMesh = TheResourceManager->CreateMesh();
  boxMesh->SetIndices(boxIndices);
  BoxMeshOp = StaticMeshNode::Create(boxMesh);

  Mesh* textureMesh = TheResourceManager->CreateMesh();
  textureMesh->SetIndices(boxIndices);
  TexturedBoxMeshOp = StaticMeshNode::Create(textureMesh);

  /// Models
  //SolidLineModel = RenderableNode::Create(SolidColorOp, LineMeshOp);
  SolidLineModel = new Drawable();
  SolidLineModel->mMaterial.Connect(&SolidColorOp);
  SolidLineModel->mMesh.Connect(LineMeshOp);

  //SolidRectModel = RenderableNode::Create(SolidColorOp, RectMeshOp);
  SolidRectModel = new Drawable();
  SolidRectModel->mMaterial.Connect(&SolidColorOp);
  SolidRectModel->mMesh.Connect(RectMeshOp);

  //SolidBoxModel = RenderableNode::Create(SolidColorOp, BoxMeshOp);
  SolidBoxModel = new Drawable();
  SolidBoxModel->mMaterial.Connect(&SolidColorOp);
  SolidBoxModel->mMesh.Connect(BoxMeshOp);

  //TexturedBoxModel = RenderableNode::Create(SolidTextureOp, TexturedBoxMeshOp);
  TexturedBoxModel = new Drawable();
  TexturedBoxModel->mMaterial.Connect(&SolidColorOp);
  TexturedBoxModel->mMesh.Connect(TexturedBoxMeshOp);

  //TextBoxModel = RenderableNode::Create(TextTextureOp, TexturedBoxMeshOp);
  TextBoxModel = new Drawable();
  TextBoxModel->mMaterial.Connect(&TextTextureOp);
  TextBoxModel->mMesh.Connect(TexturedBoxMeshOp);

  // Renderstate
  CanvasRenderstate.DepthTest = false;
  CanvasRenderstate.Face = RenderState::FACE_FRONT_AND_BACK;
  CanvasRenderstate.BlendMode = RenderState::BLEND_ALPHA;
}

UiPainter::~UiPainter() {
  SafeDelete(LineMeshOp);
  SafeDelete(SolidLineModel);
}

void UiPainter::DrawLine(float x1, float y1, float x2, float y2) {
  VertexPos vertices[] = {{Vec3(x1 + 0.5f, y1 + 0.5f, 0)}, {Vec3(x2 + 0.5f, y2 + 0.5f, 0)}};
  LineMeshOp->GetMesh()->SetVertices(vertices);
  SolidLineModel->Draw(&mGlobals, PRIMITIVE_LINES);
}

void UiPainter::DrawLine(const Vec2& From, const Vec2& To) {
  VertexPos vertices[] = {
    {Vec3(From.x + 0.5f, From.y + 0.5f, 0)},
    {Vec3(To.x + 0.5f, To.y + 0.5f, 0)}};
  LineMeshOp->GetMesh()->SetVertices(vertices);
  SolidLineModel->Draw(&mGlobals, PRIMITIVE_LINES);
}

void UiPainter::DrawRect(const Vec2& TopLeft, const Vec2& Size) {
  Vec3 pos(TopLeft.x + 0.5f, TopLeft.y + 0.5f, 0);
  VertexPos vertices[] = {
    {pos},
    {pos + Vec3(Size.x - 1, 0, 0)},
    {pos + Vec3(Size.x - 1, Size.y - 1, 0)},
    {pos + Vec3(0, Size.y - 1, 0)},
    {pos},
  };
  RectMeshOp->GetMesh()->SetVertices(vertices);
  SolidRectModel->Draw(&mGlobals, PRIMITIVE_LINE_STRIP);
}


void UiPainter::DrawBox(const Vec2& TopLeft, const Vec2& Size) {
  VertexPos vertices[] = {
    {Vec3(TopLeft.x, TopLeft.y, 0)},
    {Vec3(TopLeft.x + Size.x, TopLeft.y, 0)},
    {Vec3(TopLeft.x, TopLeft.y + Size.y, 0)},
    {Vec3(TopLeft.x + Size.x, TopLeft.y + Size.y, 0)},
  };
  BoxMeshOp->GetMesh()->SetVertices(vertices);
  SolidBoxModel->Draw(&mGlobals, PRIMITIVE_TRIANGLES);
}

void UiPainter::DrawTexture(Texture* Tex, float x, float y) {
  float w(Tex->mWidth);
  float h(Tex->mHeight);
  VertexPosUV vertices[] = {
    {Vec3(x, y, 0), Vec2(0, 0)},
    {Vec3(x + w, y, 0), Vec2(1, 0)},
    {Vec3(x, y + h, 0), Vec2(0, 1)},
    {Vec3(x + w, y + h, 0), Vec2(1, 1)},
  };

  TexOp.Set(Tex);
  TexturedBoxMeshOp->GetMesh()->SetVertices(vertices);
  TexturedBoxModel->Draw(&mGlobals);
}


void UiPainter::DrawTextTexture(TextTexture* Tex, const Vec2& Position) {
  float w = Tex->TextSize.width();
  float h = Tex->TextSize.height();
  float u = w / float(Tex->TheTexture->mWidth);
  float v = h / float(Tex->TheTexture->mHeight);
  VertexPosUV vertices[] = {
    {Vec3(Position.x, Position.y, 0), Vec2(0, 0)},
    {Vec3(Position.x + w, Position.y, 0), Vec2(u, 0)},
    {Vec3(Position.x, Position.y + h, 0), Vec2(0, v)},
    {Vec3(Position.x + w, Position.y + h, 0), Vec2(u, v)},
  };

  TexOp.Set(Tex->TheTexture);
  TexturedBoxMeshOp->GetMesh()->SetVertices(vertices);
  TextBoxModel->Draw(&mGlobals);
}


void UiPainter::Set(int Width, int Height) {
  TheDrawingAPI->SetViewport(0, 0, Width, Height);
  TheDrawingAPI->SetRenderState(&CanvasRenderstate);
  Color.Set(Vec4(1, 1, 1, 1));

  mGlobals.RenderTargetSize = Vec2(Width, Height);
  mGlobals.RenderTargetSizeRecip = Vec2(1.0f / float(Width), 1.0f / float(Height));

  mGlobals.Transformation = Matrix::Ortho(0, 0, Width, Height);
}
