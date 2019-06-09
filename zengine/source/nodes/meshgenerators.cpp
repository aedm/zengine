#include <include/nodes/meshgenerators.h>

REGISTER_NODECLASS(CubeMeshNode, "Cube");
REGISTER_NODECLASS(HalfCubeMeshNode, "HalfCube");

CubeMeshNode::CubeMeshNode()
  : MeshNode()
  , mSizeX(this, "SizeX")
  , mSizeY(this, "SizeY")
  , mSizeZ(this, "SizeZ")
{}

void CubeMeshNode::Operate() {
  if (!mMesh) mMesh = make_shared<Mesh>();

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
  case MessageType::SLOT_CONNECTION_CHANGED:
    if (mIsUpToDate) {
      mIsUpToDate = false;
      SendMsg(MessageType::NEEDS_REDRAW);
    }
    break;
  default: break;
  }
}

HalfCubeMeshNode::HalfCubeMeshNode()
  : MeshNode()
{}

void HalfCubeMeshNode::Operate() {
  if (!mMesh) mMesh = make_shared<Mesh>();

  VertexPosNorm vertices[] = {
    /// Top
    { Vec3(0.5f, 1, 0.5f), Vec3(0, 1, 0) },
    { Vec3(-0.5f, 1, 0.5f), Vec3(0, 1, 0) },
    { Vec3(0.5f, 1, -0.5f), Vec3(0, 1, 0) },
    { Vec3(-0.5f, 1, -0.5f), Vec3(0, 1, 0) },

    /// Side
    { Vec3(0.5f, 1, 0.5f), Vec3(1, 0, 0) },
    { Vec3(0.5f, 1, -0.5f), Vec3(1, 0, 0) },
    { Vec3(0.5f, 0, 0.5f), Vec3(1, 0, 0) },
    { Vec3(0.5f, 0, -0.5f), Vec3(1, 0, 0) },

    /// Front
    { Vec3(0.5f, 0, 0.5f), Vec3(0, 0, 1) },
    { Vec3(-0.5f, 0, 0.5f), Vec3(0, 0, 1) },
    { Vec3(0.5f, 1, 0.5f), Vec3(0, 0, 1) },
    { Vec3(-0.5f, 1, 0.5f), Vec3(0, 0, 1) },
  };

  IndexEntry indexes[3 * 2 * 3];
  int a = 0;
  for (int i = 0; i < 3; i++) {
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

void HalfCubeMeshNode::HandleMessage(Message* message) {
  switch (message->mType) {
  case MessageType::VALUE_CHANGED:
  case MessageType::SLOT_CONNECTION_CHANGED:
    if (mIsUpToDate) {
      mIsUpToDate = false;
      SendMsg(MessageType::NEEDS_REDRAW);
    }
    break;
  default: break;
  }
}

