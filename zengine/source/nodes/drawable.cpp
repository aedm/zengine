#include <include/nodes/drawable.h>

REGISTER_NODECLASS(Drawable, "Drawable");

Drawable::Drawable()
  : mMesh(this, "Mesh")
  , mMaterial(this, "Material")
  , mMove(this, "Move", false, true, true, -100.0f, 100.0f)
  , mRotate(this, "Rotate", false, true, true, -Pi, Pi)
  , mChildren(this, "Children", true)
  , mScale(this, "Scale")
  , mInstances(this, "Instances")
{
  mInstances.SetDefaultValue(1);
}

Drawable::~Drawable() {}

void Drawable::Draw(Globals* oldGlobals, PassType passType, PrimitiveTypeEnum Primitive) {
  auto& material = mMaterial.GetNode();
  auto& meshNode = mMesh.GetNode();
  Globals globals = *oldGlobals;

  if (mChildren.GetMultiNodeCount() == 0 && !(material && meshNode)) return;

  Vec3 movv = mMove.Get();
  if (movv.x != 0 || movv.y != 0 || movv.z != 0) {
    Matrix move = Matrix::Translate(mMove.Get());
    globals.World = globals.World * move;
  }
  Vec3 rotv = mRotate.Get();
  if (rotv.x != 0 || rotv.y != 0 || rotv.z != 0) {
    Matrix rotate = Matrix::Rotate(Quaternion::FromEuler(rotv.x, rotv.y, rotv.z));
    globals.World = globals.World * rotate;
  }
  float scalev = mScale.Get();
  if (scalev != 0.0f) {
    float s = powf(2.0f, scalev);
    Matrix scale = Matrix::Scale(Vec3(s, s, s));
    globals.World = globals.World * scale;
  }

  globals.View = globals.Camera * globals.World;
  globals.Transformation = globals.Projection * globals.View;
  globals.SkylightTransformation =
    globals.SkylightProjection * (globals.SkylightCamera * globals.World);

  if (material && meshNode) {
    meshNode->Update();
    const shared_ptr<Mesh>& mesh = meshNode->GetMesh();

    /// Set pass (pipeline state)
    auto& pass = material->GetPass(passType);
    if (!pass) return;
    pass->Update();

    if (pass->isComplete() && mesh != nullptr) {
      pass->Set(&globals);
      mesh->Render(pass->GetUsedAttributes(), UINT(mInstances.Get()), Primitive);
    }
  }

  for (UINT i = 0; i < mChildren.GetMultiNodeCount(); i++) {
    PointerCast<Drawable>(mChildren.GetReferencedMultiNode(i))->Draw(&globals, passType);
  }
}

void Drawable::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::SLOT_CONNECTION_CHANGED:
  case MessageType::VALUE_CHANGED:
    EnqueueMessage(MessageType::NEEDS_REDRAW);
    break;
  default: break;
  }
}
