#include <include/nodes/meshgenerators.h>
#include <include/resources/resourcemanager.h>

REGISTER_NODECLASS(CubeMeshNode, "Cube");

static SharedString CubeSizeXSlotName = make_shared<string>("SizeX");
static SharedString CubeSizeYSlotName = make_shared<string>("SizeY");
static SharedString CubeSizeZSlotName = make_shared<string>("SizeZ");

CubeMeshNode::CubeMeshNode() 
  : MeshNode()
  , mSizeX(this, CubeSizeXSlotName)
  , mSizeY(this, CubeSizeYSlotName)
  , mSizeZ(this, CubeSizeZSlotName)
{}

void CubeMeshNode::Operate() {
  if (!mMesh) mMesh = TheResourceManager->CreateMesh();

  float x = mSizeX.Get();
  float y = mSizeY.Get();
  float z = mSizeZ.Get();

  VertexPosUVNorm vertices[] = {
    {Vec3(x, y, z), Vec2(0, 0), Vec3(1, 0, 0)},
    {Vec3(x, -y, z), Vec2(1, 0), Vec3(1, 0, 0)},
    {Vec3(x, y, -z), Vec2(0, 1), Vec3(1, 0, 0)},
    {Vec3(x, -y, -z), Vec2(1, 1), Vec3(1, 0, 0)},

    {Vec3(-x, y, -z), Vec2(0, 1), Vec3(-1, 0, 0)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(-1, 0, 0)},
    {Vec3(-x, y, z), Vec2(0, 0), Vec3(-1, 0, 0)},
    {Vec3(-x, -y, z), Vec2(1, 0), Vec3(-1, 0, 0)},

    {Vec3(x, y, z), Vec2(0, 0), Vec3(0, 1, 0)},
    {Vec3(x, y, -z), Vec2(0, 1), Vec3(0, 1, 0)},
    {Vec3(-x, y, z), Vec2(1, 0), Vec3(0, 1, 0)},
    {Vec3(-x, y, -z), Vec2(1, 1), Vec3(0, 1, 0)},

    {Vec3(x, -y, -z), Vec2(0, 1), Vec3(0, -1, 0)},
    {Vec3(x, -y, z), Vec2(0, 0), Vec3(0, -1, 0)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(0, -1, 0)},
    {Vec3(-x, -y, z), Vec2(1, 0), Vec3(0, -1, 0)},

    {Vec3(x, -y, z), Vec2(0, 1), Vec3(0, 0, 1)},
    {Vec3(x, y, z), Vec2(0, 0), Vec3(0, 0, 1)},
    {Vec3(-x, -y, z), Vec2(1, 1), Vec3(0, 0, 1)},
    {Vec3(-x, y, z), Vec2(1, 0), Vec3(0, 0, 1)},

    {Vec3(x, y, -z), Vec2(0, 0), Vec3(0, 0, -1)},
    {Vec3(x, -y, -z), Vec2(0, 1), Vec3(0, 0, -1)},
    {Vec3(-x, y, -z), Vec2(1, 0), Vec3(0, 0, -1)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(0, 0, -1)},
  };

  IndexEntry indexes[3 * 2 * 6];
  int a = 0;
  for (int i = 0; i < 6; i++) {
    int base = i * 4;
    indexes[a++] = 0 + base;
    indexes[a++] = 1 + base;
    indexes[a++] = 2 + base;
    indexes[a++] = 2 + base;
    indexes[a++] = 1 + base;
    indexes[a++] = 3 + base;
  }

  mMesh->SetVertices(vertices);
  mMesh->SetIndices(indexes);
}

void CubeMeshNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  Node::HandleMessage(message, slot, payload);
  switch (message) {
    case NodeMessage::VALUE_CHANGED:
      if (mIsUpToDate) {
        mIsUpToDate = false;
        SendMsg(NodeMessage::VALUE_CHANGED);
      }
      break;
    default: break;
  }
}
