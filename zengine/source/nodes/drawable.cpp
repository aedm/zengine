#include <include/nodes/drawable.h>

REGISTER_NODECLASS(Drawable, "Drawable");

static SharedString MaterialSlotName = make_shared<string>("Material");
static SharedString MeshSlotName = make_shared<string>("Mesh");
static SharedString MoveSlotName = make_shared<string>("Move");
static SharedString RotateSlotName = make_shared<string>("Rotate");
static SharedString ChildrenSlotName = make_shared<string>("Children");
static SharedString InstancesSlotName = make_shared<string>("Instances");

Drawable::Drawable()
  : Node(NodeType::DRAWABLE)
  , mMesh(this, MeshSlotName)
  , mMaterial(this, MaterialSlotName) 
  , mMove(this, MoveSlotName)
  , mRotate(this, RotateSlotName)
  , mChildren(this, ChildrenSlotName, true)
  , mInstances(this, InstancesSlotName)
{
  mInstances.SetDefaultValue(1);
}

Drawable::~Drawable() {}

void Drawable::Draw(Globals* oldGlobals, PassType passType, PrimitiveTypeEnum Primitive) {
  Material* material = mMaterial.GetNode();
  MeshNode* meshNode = mMesh.GetNode();
  Globals globals = *oldGlobals;

  if (mChildren.GetMultiNodes().size() == 0 && !(material && meshNode)) return;

  Vec3 rotv = mRotate.Get();
  if (rotv.x != 0 || rotv.y != 0 || rotv.z != 0) {
    Matrix rotate = Matrix::Rotate(Quaternion::FromEuler(rotv.x, rotv.y, rotv.z));
    globals.World = globals.World * rotate;
  }
  Vec3 movv = mMove.Get();
  if (movv.x != 0 || movv.y != 0 || movv.z != 0) {
    Matrix move = Matrix::Translate(mMove.Get());
    globals.World = globals.World * move;
  }
  
  globals.View = globals.Camera * globals.World;
  globals.Transformation = globals.Projection * globals.View;
  globals.SkylightTransformation = 
    globals.SkylightProjection * (globals.SkylightCamera * globals.World);

  if (material && meshNode) {
    meshNode->Update();
    const Mesh* mesh = meshNode->GetMesh();

    /// Set pass (pipeline state)
    Pass* pass = material->GetPass(passType);
    if (!pass) return;
    pass->Update();

    if (pass->isComplete()) {
      pass->Set(&globals);
      mesh->Render(pass->GetUsedAttributes(), UINT(mInstances.Get()), Primitive);
    }
  }

  for (Node* node : mChildren.GetMultiNodes()) {
    static_cast<Drawable*>(node)->Draw(&globals, passType);
  }
}

void Drawable::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::VALUE_CHANGED:
      ReceiveMessage(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}
