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

  VertexPosUVNormTangent vertices[] = {
    {Vec3(x, y, z), Vec2(0, 0), Vec3(1, 0, 0)},
    {Vec3(x, y, -z), Vec2(0, 1), Vec3(1, 0, 0)},
    {Vec3(x, -y, z), Vec2(1, 0), Vec3(1, 0, 0)},
    {Vec3(x, -y, -z), Vec2(1, 1), Vec3(1, 0, 0)},

    {Vec3(-x, y, -z), Vec2(0, 1), Vec3(-1, 0, 0)},
    {Vec3(-x, y, z), Vec2(0, 0), Vec3(-1, 0, 0)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(-1, 0, 0)},
    {Vec3(-x, -y, z), Vec2(1, 0), Vec3(-1, 0, 0)},

    {Vec3(x, y, z), Vec2(0, 0), Vec3(0, 1, 0)},
    {Vec3(-x, y, z), Vec2(1, 0), Vec3(0, 1, 0)},
    {Vec3(x, y, -z), Vec2(0, 1), Vec3(0, 1, 0)},
    {Vec3(-x, y, -z), Vec2(1, 1), Vec3(0, 1, 0)},

    {Vec3(x, -y, -z), Vec2(0, 1), Vec3(0, -1, 0)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(0, -1, 0)},
    {Vec3(x, -y, z), Vec2(0, 0), Vec3(0, -1, 0)},
    {Vec3(-x, -y, z), Vec2(1, 0), Vec3(0, -1, 0)},

    {Vec3(x, -y, z), Vec2(0, 1), Vec3(0, 0, 1)},
    {Vec3(-x, -y, z), Vec2(1, 1), Vec3(0, 0, 1)},
    {Vec3(x, y, z), Vec2(0, 0), Vec3(0, 0, 1)},
    {Vec3(-x, y, z), Vec2(1, 0), Vec3(0, 0, 1)},

    {Vec3(x, y, -z), Vec2(0, 0), Vec3(0, 0, -1)},
    {Vec3(-x, y, -z), Vec2(1, 0), Vec3(0, 0, -1)},
    {Vec3(x, -y, -z), Vec2(0, 1), Vec3(0, 0, -1)},
    {Vec3(-x, -y, -z), Vec2(1, 1), Vec3(0, 0, -1)},
  };

  Vec3 tangents[] = {
    Vec3(0, -1, 0),
    Vec3(0, -1, 0),
    Vec3(-1, 0, 0),
    Vec3(-1, 0, 0),
    Vec3(-1, 0, 0),
    Vec3(-1, 0, 0),
  };

  for (int i = 0; i < 6; i++) {
    for (int o = 0; o < 4; o++) {
      vertices[i * 4 + o].tangent = tangents[i];
    }
  }

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

void CubeMeshNode::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::VALUE_CHANGED:
      if (mIsUpToDate) {
        mIsUpToDate = false;
        SendMsg(MessageType::VALUE_CHANGED);
      }
      break;
    default: break;
  }
}
