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

void Drawable::ApplyTransformation(Globals& globals) const
{
  const vec3 movv = mMove.Get();
  if (movv.x != 0 || movv.y != 0 || movv.z != 0) {
    const mat4 move = glm::translate(mat4(1.0f), mMove.Get());
    globals.World = move * globals.World;
  }
  const vec3 rotv = mRotate.Get();
  if (rotv.x != 0 || rotv.y != 0 || rotv.z != 0) {
    const mat4 rotateX = glm::rotate(mat4(1.0f), rotv.x, { 1, 0, 0 });
    const mat4 rotateY = glm::rotate(rotateX, rotv.y, { 0, 1, 0 });
    const mat4 rotateZ = glm::rotate(rotateY, rotv.z, { 0, 0, 1 });
    globals.World = rotateZ * globals.World;
  }
  const float scalev = mScale.Get();
  if (scalev != 0.0f) {
    const float s = powf(2.0f, scalev);
    const mat4 scale = glm::scale(mat4(1.0f), {s, s, s});
    globals.World = scale * globals.World;
  }

  globals.View = globals.World * globals.Camera;
  globals.Transformation = globals.View * globals.Projection;
  globals.SkylightTransformation =
     globals.World * globals.SkylightCamera * globals.SkylightProjection;
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
