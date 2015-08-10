#include <include/nodes/drawable.h>

REGISTER_NODECLASS(Drawable, "Drawable");

static SharedString MaterialSlotName = make_shared<string>("Material");
static SharedString MeshSlotName = make_shared<string>("Mesh");

Drawable::Drawable()
  : Node(NodeType::DRAWABLE)
  , mMesh(this, MeshSlotName)
  , mMaterial(this, MaterialSlotName) 
{}

Drawable::~Drawable() {}

void Drawable::Draw(Globals* globals, PrimitiveTypeEnum Primitive) {
  if (!mIsProperlyConnected) return;
  Material* material = mMaterial.GetNode();
  if (!material) return;
  const Mesh* mesh = mMesh.GetNode()->GetMesh();

  /// Set pass (pipeline state)
  Pass* pass = material->GetPass();
  if (!pass) return;
  pass->Update();

  if (!pass->isComplete()) return;
  pass->Set(globals);

  /// Set vertex buffer and attributes
  TheDrawingAPI->SetVertexBuffer(mesh->mVertexHandle);
  for (ShaderAttributeDesc desc : pass->GetUsedAttributes()) {
    VertexAttribute* attribute = mesh->mFormat->mAttributesArray[(UINT)desc.Usage];
    if (attribute != nullptr) {
      TheDrawingAPI->EnableVertexAttribute(desc.Handle,
          gVertexAttributeType[(UINT)desc.Usage], attribute->Offset,
          mesh->mFormat->mStride);
    } else {
      SHOULDNT_HAPPEN;
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

void Drawable::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
    case NodeMessage::NEEDS_REDRAW:
      SendMsg(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}
