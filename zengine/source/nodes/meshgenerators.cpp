#include <include/nodes/meshgenerators.h>

REGISTER_NODECLASS(CubeMeshNode, "Cube");
REGISTER_NODECLASS(HalfCubeMeshNode, "HalfCube");
REGISTER_NODECLASS(GeosphereMeshNode, "Geosphere");

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

GeosphereMeshNode::GeosphereMeshNode()
  : MeshNode()
  , mResolution(this, "Resolution", false, true, true, 0.0f, 8.0f)
  , mSize(this, "Size", false, true, true, 0.0f, 10.0f)
  , mFlatten(this, "Flatten", false, true, true, 0.0f, 1.0f)
{
  mResolution.SetDefaultValue(5);
  mSize.SetDefaultValue(1.0f);
  mFlatten.SetDefaultValue(0);
}


void MakeGeosphereTriangle(const Vec3& p1, const Vec3& p2, const Vec3& p3, float flatten,
  float size, int resolution, VertexPosUVNormTangent** oVertexTarget,
  IndexEntry** oIndexTarget, int* oBaseIndex)
{
  /// Generate vertices
  Vec3 d1 = p2 - p1;
  Vec3 d2 = p3 - p1;
  Vec3 flatNormal = d2.Cross(d1).Normal();
  Vec3 upVector(0, 1, 0);

  VertexPosUVNormTangent* vertexTarget = *oVertexTarget;
  int maxCoord = 1 << resolution;
  float vRecip = 1.0f / float(maxCoord);
  for (int y = 0; y <= maxCoord; y++) {
    float yr = float(y) * vRecip;
    for (int x = 0; x <= maxCoord - y; x++) {
      float xr = float(x) * vRecip;
      Vec3 p = p1 + d1 * xr + d2 * yr;
      Vec3 spherical = p.Normal();
      Vec3 pos = (spherical + (p - spherical) * flatten) * size;
      Vec3 normal = spherical + (flatNormal - spherical) * flatten;
      Vec3 tangent = upVector.Cross(normal).Normal();
      float urad = atan2f(spherical.x, spherical.z);
      float xz = sqrtf(spherical.x * spherical.x + spherical.z * spherical.z);
      float vrad = atan2f(xz, spherical.y);
      Vec2 uv((urad / Pi) * 0.5f + 0.5f, vrad / Pi + 0.5f);
      vertexTarget->position = pos;
      vertexTarget->uv = uv;
      vertexTarget->normal = normal;
      vertexTarget->tangent = tangent;
      vertexTarget++;
    }
  }
  *oVertexTarget = vertexTarget;

  const int ox[4] = { 0, 0, 1, 1 };
  const int oy[4] = { 0, 1, 1, 0 };
  const int dir[4] = { 1, 1, -1, 1 };

  IndexEntry* indexTarget = *oIndexTarget;

  /// Generate indices
  int baseIndex = *oBaseIndex;
  int trianglesPerSide = 1 << (resolution * 2);
  int verticesPerSide = (maxCoord + 1) * (maxCoord + 2) / 2;
  for (int i = 0; i < trianglesPerSide; i++) {
    /// Cache-friendlier ordering for really high resolution meshes
    int k = i;
    int x1 = 0, y1 = 0, x2 = 1, y2 = 0, x3 = 0, y3 = 1, step = 1;
    while (k > 0) {
      int sel = k & 3;
      x1 = x1 * dir[sel] + step * ox[sel];
      y1 = y1 * dir[sel] + step * oy[sel];
      x2 = x2 * dir[sel] + step * ox[sel];
      y2 = y2 * dir[sel] + step * oy[sel];
      x3 = x3 * dir[sel] + step * ox[sel];
      y3 = y3 * dir[sel] + step * oy[sel];
      step <<= 1;
      k >>= 2;
    }
    int index1 = x1 + verticesPerSide - ((maxCoord - y1 + 1) * (maxCoord - y1 + 2) / 2);
    int index2 = x2 + verticesPerSide - ((maxCoord - y2 + 1) * (maxCoord - y2 + 2) / 2);
    int index3 = x3 + verticesPerSide - ((maxCoord - y3 + 1) * (maxCoord - y3 + 2) / 2);

    indexTarget[0] = index1 + baseIndex;
    indexTarget[1] = index2 + baseIndex;
    indexTarget[2] = index3 + baseIndex;
    indexTarget += 3;
  }
  *oIndexTarget = indexTarget;
  *oBaseIndex += verticesPerSide;
}


void GeosphereMeshNode::Operate() {
  if (!mMesh) mMesh = make_shared<Mesh>();

  int resolution = int(mResolution.Get());
  float size = mSize.Get();
  float flatten = mFlatten.Get();

  int vertexPerEdge = 1 + (1 << resolution);
  int vertexPerSide = vertexPerEdge * (vertexPerEdge + 1) / 2;
  int vertexCount = 4 * vertexPerSide;

  int trianglesPerSide = 1 << (resolution * 2);
  int indexCount = trianglesPerSide * 3 * 4;

  vector<VertexPosUVNormTangent> vertices(vertexCount);
  vector<IndexEntry> indices(indexCount);

  VertexPosUVNormTangent* vertexTarget = &vertices[0];
  IndexEntry* indexTarget = &indices[0];

  float p = 1.0f / sqrtf(3.0f);
  /// Tetraeder vertices
  Vec3 tv[] = {
    Vec3(1, 1, 1) * p,
    Vec3(1, -1, -1) * p,
    Vec3(-1, 1, -1) * p,
    Vec3(-1, -1, 1) * p,
  };

  int baseIndex = 0;
  MakeGeosphereTriangle(tv[0], tv[2], tv[1], flatten, size, resolution,
    &vertexTarget, &indexTarget, &baseIndex);
  MakeGeosphereTriangle(tv[0], tv[3], tv[2], flatten, size, resolution,
    &vertexTarget, &indexTarget, &baseIndex);
  MakeGeosphereTriangle(tv[0], tv[1], tv[3], flatten, size, resolution,
    &vertexTarget, &indexTarget, &baseIndex);
  MakeGeosphereTriangle(tv[1], tv[2], tv[3], flatten, size, resolution,
    &vertexTarget, &indexTarget, &baseIndex);

  mMesh->AllocateVertices(VertexPosUVNormTangent::format, vertexCount);
  mMesh->AllocateIndices(indexCount);

  mMesh->UploadVertices(&vertices[0]);
  mMesh->UploadIndices(&indices[0]);
}

void GeosphereMeshNode::HandleMessage(Message* message) {
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