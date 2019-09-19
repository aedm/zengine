#include <include/nodes/drawable.h>

REGISTER_NODECLASS(Drawable, "Drawable");

Drawable::Drawable()
  : mMesh(this, "Mesh")
  , mMaterial(this, "Material")
  , mMove(this, "Move", false, true, true, -10.0f, 10.0f)
  , mRotate(this, "Rotate", false, true, true, -Pi, Pi)
  , mChildren(this, "Children", true)
  , mScale(this, "Scale")
  , mInstances(this, "Instances")
  , mIsShadowCenter(this, "Force shadow center")
{
  mInstances.SetDefaultValue(1);
}

Drawable::~Drawable() = default;

void Drawable::Draw(Globals* oldGlobals, PassType passType, PrimitiveTypeEnum Primitive) {
  auto& material = mMaterial.GetNode();
  auto& meshNode = mMesh.GetNode();

  if (mChildren.GetMultiNodeCount() == 0 && !(material && meshNode)) return;
  Globals globals = *oldGlobals;
  ApplyTransformation(globals);

  if (material && meshNode) {
    meshNode->Update();
    const shared_ptr<Mesh>& mesh = meshNode->GetMesh();

    /// Set pass (pipeline state)
    auto& pass = material->GetPass(passType);
    if (!pass) return;
    pass->Update();

    if (pass->isComplete() && mesh != nullptr) {
      pass->Set(&globals);
      mesh->Render(UINT(mInstances.Get()), Primitive);
    }
  }

  for (UINT i = 0; i < mChildren.GetMultiNodeCount(); i++) {
    PointerCast<Drawable>(mChildren.GetReferencedMultiNode(i))->Draw(&globals, passType);
  }
}


void Drawable::ComputeForcedShadowCenter(Globals* globals, Vec3& oShadowCenter) const
{
  Globals currentGlobals = *globals;
  ApplyTransformation(currentGlobals);
  if (mIsShadowCenter.Get() > 0.5f) {
    const Vec4 s = Vec4(0, 0, 0, 1) * currentGlobals.View;
    oShadowCenter = Vec3(s.x, s.y, s.z);
  }
  for (UINT i = 0; i < mChildren.GetMultiNodeCount(); i++) {
    PointerCast<Drawable>(mChildren.GetReferencedMultiNode(i))->ComputeForcedShadowCenter(
      &currentGlobals, oShadowCenter);
  }
}

void Drawable::ApplyTransformation(Globals& globals) const
{
  const Vec3 movv = mMove.Get();
  if (movv.x != 0 || movv.y != 0 || movv.z != 0) {
    const Matrix move = Matrix::Translate(mMove.Get());
    globals.World = globals.World * move;
  }
  const Vec3 rotv = mRotate.Get();
  if (rotv.x != 0 || rotv.y != 0 || rotv.z != 0) {
    const Matrix rotate = Matrix::Rotate(Quaternion::FromEuler(rotv.x, rotv.y, rotv.z));
    globals.World = globals.World * rotate;
  }
  const float scalev = mScale.Get();
  if (scalev != 0.0f) {
    const float s = powf(2.0f, scalev);
    const Matrix scale = Matrix::Scale(Vec3(s, s, s));
    globals.World = globals.World * scale;
  }

  globals.View = globals.Camera * globals.World;
  globals.Transformation = globals.Projection * globals.View;
  globals.SkylightTransformation =
    globals.SkylightProjection * (globals.SkylightCamera * globals.World);
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
