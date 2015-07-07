#include <include/nodes/drawable.h>


Drawable::Drawable()
  : Node(NodeType::PASS)
  , mMesh(this, make_shared<string>("Mesh"))
  , mMaterial(NodeType::MATERIAL, this, make_shared<string>("Material")) {}

Drawable::~Drawable() {}

void Drawable::Draw(Globals* globals, PrimitiveTypeEnum Primitive) {
  if (!mIsProperlyConnected) return;
  Material* material = static_cast<Material*>(mMaterial.GetNode());
  const Mesh* mesh = mMesh.GetMesh();

  /// Set pass (pipeline state)
  Pass* pass = material->GetPass();
  if (!pass || !pass->isComplete()) return;
  pass->Set(globals);

  /// Set vertex buffer and attributes
  TheDrawingAPI->SetVertexBuffer(mesh->mVertexHandle);
  for (ShaderAttributeDesc desc : pass->GetUsedAttributes()) {
    VertexAttribute* attribute = mesh->mFormat->mAttributesArray[(UINT)desc.Usage];
    if (attribute != nullptr) {
      TheDrawingAPI->EnableVertexAttribute(desc.Handle,
          gVertexAttributeType[(UINT)desc.Usage], attribute->Offset,
          mesh->mFormat->mStride);
    }
  }

  /// TODO: set output buffers

  /// Render mesh
  if (mesh->mIndexHandle) {
    TheDrawingAPI->Render(mesh->mIndexHandle, mesh->mIndexCount, Primitive);
  } else {
    TheDrawingAPI->Render(0, mesh->mVertexCount, Primitive);
  }
}