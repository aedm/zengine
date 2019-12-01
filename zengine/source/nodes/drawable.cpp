#include <include/nodes/drawable.h>
#include <glm/gtc/matrix_transform.hpp>

REGISTER_NODECLASS(Drawable, "Drawable");

Drawable::Drawable()
  : mMesh(this, "Mesh")
  , mMaterial(this, "Material")
  , mChildren(this, "Children", true)
  , mMove(this, "Move", false, true, true, -10.0f, 10.0f)
  , mRotate(this, "Rotate", false, true, true, -Pi, Pi)
  , mScale(this, "Scale")
  , mInstances(this, "Instances")
  , mIsShadowCenter(this, "Force shadow center")
{
  mInstances.SetDefaultValue(1);
}

Drawable::~Drawable() = default;

void Drawable::Draw(Globals* oldGlobals, PassType passType, PrimitiveTypeEnum Primitive) {
  const auto& material = mMaterial.GetNode();
  const auto& meshNode = mMesh.GetNode();

  if (mChildren.GetMultiNodeCount() == 0 && !(material && meshNode)) return;
  Globals globals = *oldGlobals;
  ApplyTransformation(globals);

  if (material && meshNode) {
    meshNode->Update();
    const std::shared_ptr<Mesh>& mesh = meshNode->GetMesh();

    /// Set pass (pipeline state)
    const auto& pass = material->GetPass(passType);
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


void Drawable::ComputeForcedShadowCenter(Globals* globals, vec3& oShadowCenter) const
{
  Globals currentGlobals = *globals;
  ApplyTransformation(currentGlobals);
  if (mIsShadowCenter.Get() > 0.5f) {
    const vec4 s = vec4(0, 0, 0, 1) * currentGlobals.View;
    oShadowCenter = vec3(s.x, s.y, s.z);
  }
  for (UINT i = 0; i < mChildren.GetMultiNodeCount(); i++) {
    PointerCast<Drawable>(mChildren.GetReferencedMultiNode(i))->ComputeForcedShadowCenter(
      &currentGlobals, oShadowCenter);
  }
}

void Drawable:: ApplyTransformation(Globals& globals) const
{
  const vec3 move = mMove.Get();
  if (move.x != 0 || move.y != 0 || move.z != 0) {
    globals.World = glm::translate(globals.World, mMove.Get());
  }
  const vec3 rotate = mRotate.Get();
  if (rotate.x != 0 || rotate.y != 0 || rotate.z != 0) {
    globals.World = glm::rotate(globals.World, rotate.x, { 1, 0, 0 });
    globals.World = glm::rotate(globals.World, rotate.y, { 0, 1, 0 });
    globals.World = glm::rotate(globals.World, rotate.z, { 0, 0, 1 });
  }
  const float scale = mScale.Get();
  if (scale != 0.0f) {
    const float s = powf(2.0f, scale);
    globals.World = glm::scale(globals.World, { s, s, s });
  }

  globals.View = globals.Camera * globals.World;
  globals.Transformation = globals.Projection * globals.Camera * globals.World;
  globals.SkylightTransformation =
    globals.SkylightProjection * globals.SkylightCamera * globals.World;
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
