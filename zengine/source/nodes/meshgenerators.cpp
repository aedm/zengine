#include <include/nodes/meshgenerators.h>

REGISTER_NODECLASS(CubeMeshNode, "Cube");
REGISTER_NODECLASS(HalfCubeMeshNode, "HalfCube");
REGISTER_NODECLASS(GeosphereMeshNode, "Geosphere");
REGISTER_NODECLASS(PlaneMeshNode, "Plane");
REGISTER_NODECLASS(PolarSphereMeshNode, "PolarSphere");

using glm::vec3;
using glm::vec2;

CubeMeshNode::CubeMeshNode()
  : mSizeX(this, "SizeX")
  , mSizeY(this, "SizeY")
  , mSizeZ(this, "SizeZ")
{}

void CubeMeshNode::Operate() {
  if (!mMesh) mMesh = std::make_shared<Mesh>();

  const float x = mSizeX.Get();
  const float y = mSizeY.Get();
  const float z = mSizeZ.Get();

  VertexPosUvNormTangent vertices[] = {
    {vec3(x, y, z), vec2(0, 0), vec3(1, 0, 0), vec3(0, 0, 0)},
    {vec3(x, y, -z), vec2(0, 1), vec3(1, 0, 0), vec3(0, 0, 0)},
    {vec3(x, -y, z), vec2(1, 0), vec3(1, 0, 0), vec3(0, 0, 0)},
    {vec3(x, -y, -z), vec2(1, 1), vec3(1, 0, 0), vec3(0, 0, 0)},

    {vec3(-x, y, -z), vec2(0, 1), vec3(-1, 0, 0), vec3(0, 0, 0)},
    {vec3(-x, y, z), vec2(0, 0), vec3(-1, 0, 0), vec3(0, 0, 0)},
    {vec3(-x, -y, -z), vec2(1, 1), vec3(-1, 0, 0), vec3(0, 0, 0)},
    {vec3(-x, -y, z), vec2(1, 0), vec3(-1, 0, 0), vec3(0, 0, 0)},

    {vec3(x, y, z), vec2(0, 0), vec3(0, 1, 0), vec3(0, 0, 0)},
    {vec3(-x, y, z), vec2(1, 0), vec3(0, 1, 0), vec3(0, 0, 0)},
    {vec3(x, y, -z), vec2(0, 1), vec3(0, 1, 0), vec3(0, 0, 0)},
    {vec3(-x, y, -z), vec2(1, 1), vec3(0, 1, 0), vec3(0, 0, 0)},

    {vec3(x, -y, -z), vec2(0, 1), vec3(0, -1, 0), vec3(0, 0, 0)},
    {vec3(-x, -y, -z), vec2(1, 1), vec3(0, -1, 0), vec3(0, 0, 0)},
    {vec3(x, -y, z), vec2(0, 0), vec3(0, -1, 0), vec3(0, 0, 0)},
    {vec3(-x, -y, z), vec2(1, 0), vec3(0, -1, 0), vec3(0, 0, 0)},

    {vec3(x, -y, z), vec2(0, 1), vec3(0, 0, 1), vec3(0, 0, 0)},
    {vec3(-x, -y, z), vec2(1, 1), vec3(0, 0, 1), vec3(0, 0, 0)},
    {vec3(x, y, z), vec2(0, 0), vec3(0, 0, 1), vec3(0, 0, 0)},
    {vec3(-x, y, z), vec2(1, 0), vec3(0, 0, 1), vec3(0, 0, 0)},

    {vec3(x, y, -z), vec2(0, 0), vec3(0, 0, -1), vec3(0, 0, 0)},
    {vec3(-x, y, -z), vec2(1, 0), vec3(0, 0, -1), vec3(0, 0, 0)},
    {vec3(x, -y, -z), vec2(0, 1), vec3(0, 0, -1), vec3(0, 0, 0)},
    {vec3(-x, -y, -z), vec2(1, 1), vec3(0, 0, -1), vec3(0, 0, 0)},
  };

  vec3 tangents[] = {
    vec3(0, -1, 0),
    vec3(0, -1, 0),
    vec3(-1, 0, 0),
    vec3(-1, 0, 0),
    vec3(-1, 0, 0),
    vec3(-1, 0, 0),
  };

  for (int i = 0; i < 6; i++) {
    for (int o = 0; o < 4; o++) {
      vertices[i * 4 + o].mTangent = tangents[i];
    }
  }

  IndexEntry indexes[3 * 2 * 6];
  int a = 0;
  for (int i = 0; i < 6; i++) {
    const int base = i * 4;
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

void HalfCubeMeshNode::Operate() {
  if (!mMesh) mMesh = std::make_shared<Mesh>();

  VertexPosNorm vertices[] = {
    /// Top
    { vec3(0.5f, 1, 0.5f), vec3(0, 1, 0) },
    { vec3(-0.5f, 1, 0.5f), vec3(0, 1, 0) },
    { vec3(0.5f, 1, -0.5f), vec3(0, 1, 0) },
    { vec3(-0.5f, 1, -0.5f), vec3(0, 1, 0) },

    /// Side
    { vec3(0.5f, 1, 0.5f), vec3(1, 0, 0) },
    { vec3(0.5f, 1, -0.5f), vec3(1, 0, 0) },
    { vec3(0.5f, 0, 0.5f), vec3(1, 0, 0) },
    { vec3(0.5f, 0, -0.5f), vec3(1, 0, 0) },

    /// Front
    { vec3(0.5f, 0, 0.5f), vec3(0, 0, 1) },
    { vec3(-0.5f, 0, 0.5f), vec3(0, 0, 1) },
    { vec3(0.5f, 1, 0.5f), vec3(0, 0, 1) },
    { vec3(-0.5f, 1, 0.5f), vec3(0, 0, 1) },
  };

  IndexEntry indexes[3 * 2 * 3];
  int a = 0;
  for (int i = 0; i < 3; i++) {
    const int base = i * 4;
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
  : mResolution(this, "Resolution", false, true, true, 0.0f, 8.0f)
  , mSize(this, "Size", false, true, true, 0.0f, 10.0f)
  , mFlatten(this, "Flatten", false, true, true, 0.0f, 1.0f)
{
  mResolution.SetDefaultValue(5);
  mSize.SetDefaultValue(1.0f);
  mFlatten.SetDefaultValue(0);
}


void MakeGeosphereTriangle(const vec3& p1, const vec3& p2, const vec3& p3, float flatten,
  float size, int resolution, VertexPosUvNormTangent** oVertexTarget,
  IndexEntry** oIndexTarget, int* oBaseIndex)
{
  /// Generate vertices
  const vec3 d1 = p2 - p1;
  const vec3 d2 = p3 - p1;
  const vec3 flatNormal = normalize(cross(d2, d1));
  const vec3 upVector(0, 1, 0);

  VertexPosUvNormTangent* vertexTarget = *oVertexTarget;
  const int maxCoord = 1 << resolution;
  const float vRecip = 1.0f / float(maxCoord);
  for (int y = 0; y <= maxCoord; y++) {
    const float yr = float(y) * vRecip;
    for (int x = 0; x <= maxCoord - y; x++) {
      const float xr = float(x) * vRecip;
      vec3 p = p1 + d1 * xr + d2 * yr;
      vec3 spherical = normalize(p);
      const vec3 pos = (spherical + (p - spherical) * flatten) * size;
      vec3 normal = spherical + (flatNormal - spherical) * flatten;
      const vec3 tangent = normalize(cross(upVector, normal));
      const float urad = atan2f(spherical.x, spherical.z);
      const float xz = sqrtf(spherical.x * spherical.x + spherical.z * spherical.z);
      const float vrad = atan2f(xz, spherical.y);
      const vec2 uv((urad / Pi) * 0.5f + 0.5f, vrad / Pi + 0.5f);
      vertexTarget->mPosition = pos;
      vertexTarget->mUv = uv;
      vertexTarget->mNormal = normal;
      vertexTarget->mTangent = tangent;
      vertexTarget++;
    }
  }
  *oVertexTarget = vertexTarget;

  const int ox[4] = { 0, 0, 1, 1 };
  const int oy[4] = { 0, 1, 1, 0 };
  const int dir[4] = { 1, 1, -1, 1 };

  IndexEntry* indexTarget = *oIndexTarget;

  /// Generate indices
  const int baseIndex = *oBaseIndex;
  const int trianglesPerSide = 1 << (resolution * 2);
  const int verticesPerSide = (maxCoord + 1) * (maxCoord + 2) / 2;
  for (int i = 0; i < trianglesPerSide; i++) {
    /// Cache-friendlier ordering for really high resolution meshes
    int k = i;
    int x1 = 0, y1 = 0, x2 = 1, y2 = 0, x3 = 0, y3 = 1, step = 1;
    while (k > 0) {
      const int sel = k & 3;
      x1 = x1 * dir[sel] + step * ox[sel];
      y1 = y1 * dir[sel] + step * oy[sel];
      x2 = x2 * dir[sel] + step * ox[sel];
      y2 = y2 * dir[sel] + step * oy[sel];
      x3 = x3 * dir[sel] + step * ox[sel];
      y3 = y3 * dir[sel] + step * oy[sel];
      step <<= 1;
      k >>= 2;
    }
    const int index1 = x1 + verticesPerSide - ((maxCoord - y1 + 1) * (maxCoord - y1 + 2) / 2);
    const int index2 = x2 + verticesPerSide - ((maxCoord - y2 + 1) * (maxCoord - y2 + 2) / 2);
    const int index3 = x3 + verticesPerSide - ((maxCoord - y3 + 1) * (maxCoord - y3 + 2) / 2);

    indexTarget[0] = index1 + baseIndex;
    indexTarget[1] = index2 + baseIndex;
    indexTarget[2] = index3 + baseIndex;
    indexTarget += 3;
  }
  *oIndexTarget = indexTarget;
  *oBaseIndex += verticesPerSide;
}


void GeosphereMeshNode::Operate() {
  if (!mMesh) mMesh = std::make_shared<Mesh>();

  const int resolution = int(mResolution.Get());
  const float size = mSize.Get();
  const float flatten = mFlatten.Get();

  const int vertexPerEdge = 1 + (1 << resolution);
  const int vertexPerSide = vertexPerEdge * (vertexPerEdge + 1) / 2;
  const int vertexCount = 4 * vertexPerSide;

  const int trianglesPerSide = 1 << (resolution * 2);
  const int indexCount = trianglesPerSide * 3 * 4;

  std::vector<VertexPosUvNormTangent> vertices(vertexCount);
  std::vector<IndexEntry> indices(indexCount);

  VertexPosUvNormTangent* vertexTarget = &vertices[0];
  IndexEntry* indexTarget = &indices[0];

  const float p = 1.0f / sqrtf(3.0f);
  /// Tetraeder vertices
  vec3 tv[] = {
    vec3(1, 1, 1) * p,
    vec3(1, -1, -1) * p,
    vec3(-1, 1, -1) * p,
    vec3(-1, -1, 1) * p,
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

  mMesh->AllocateVertices(VertexPosUvNormTangent::mFormat, vertexCount);
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

PlaneMeshNode::PlaneMeshNode()
  : mResolution(this, "Resolution", false, true, true, 0.0f, 8.0f)
  , mSize(this, "Size", false, true, true, 0.0f, 10.0f)
{
  mResolution.SetDefaultValue(1);
  mSize.SetDefaultValue(1.0f);
}

void PlaneMeshNode::Operate() {
  if (!mMesh) mMesh = std::make_shared<Mesh>();

  const int resolution = int(mResolution.Get());
  const float size = mSize.Get();

  const int segmentsPerEdge = 1 << resolution;
  const int verticesPerEdge = segmentsPerEdge + 1;
  const int quadCount = segmentsPerEdge * segmentsPerEdge;
  const int vertexCount = verticesPerEdge * verticesPerEdge;
  const int indexCount = quadCount * 6;

  std::vector<VertexPosUvNormTangent> vertices(vertexCount);
  std::vector<IndexEntry> indices(indexCount);

  VertexPosUvNormTangent* vertexTarget = &vertices[0];
  IndexEntry* indexTarget = &indices[0];

  /// Generate vertices
  const float uvStep = 1.0f / float(segmentsPerEdge);
  const float coordStep = uvStep * 2.0f;
  float yCoord = -1.0f;
  float v = 0.0f;
  for (int y = 0; y <= segmentsPerEdge; y++) {
    float xCoord = -1.0f;
    float u = 0.0f;
    for (int x = 0; x <= segmentsPerEdge; x++) {
      vertexTarget->mPosition = vec3(xCoord, 0.0, yCoord) * size;
      vertexTarget->mUv = vec2(u, v);
      vertexTarget->mNormal = vec3(0, 1, 0);
      vertexTarget->mTangent = vec3(1, 0, 0);
      vertexTarget++;
      xCoord += coordStep;
      u += uvStep;
    }
    yCoord += coordStep;
    v += uvStep;
  }

  /// Generate indices
  for (int i = 0; i < quadCount; i++) {
    int x = 0, y = 0;
    for (int k = 0; k < resolution; k++) {
      /// Chache-friendly tiling
      x += ((i >> (k * 2)) & 1) * (1 << k);
      y += ((i >> (k * 2 + 1)) & 1) * (1 << k);
    }
    const int p = x + y * verticesPerEdge;;
    indexTarget[0] = p;
    indexTarget[1] = p + 1;
    indexTarget[2] = p + verticesPerEdge;
    indexTarget[3] = p + verticesPerEdge;
    indexTarget[4] = p + 1;
    indexTarget[5] = p + 1 + verticesPerEdge;
    indexTarget += 6;
  }

  mMesh->AllocateVertices(VertexPosUvNormTangent::mFormat, vertexCount);
  mMesh->AllocateIndices(indexCount);

  mMesh->UploadVertices(&vertices[0]);
  mMesh->UploadIndices(&indices[0]);
}

void PlaneMeshNode::HandleMessage(Message* message) {
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


PolarSphereMeshNode::PolarSphereMeshNode()
  : mResolution(this, "Resolution", false, true, true, 0.0f, 8.0f)
  , mSize(this, "Size", false, true, true, 0.0f, 10.0f)
{
  mResolution.SetDefaultValue(5);
  mSize.SetDefaultValue(1.0f);
}

void PolarSphereMeshNode::Operate() {
  if (!mMesh) mMesh = std::make_shared<Mesh>();

  int resolution = int(mResolution.Get());
  if (resolution < 3) resolution = 3;
  const float size = mSize.Get();

  const int longAxisVertices = resolution * 2;
  const int shortAxisVertices = resolution;
  const int vertexCount = longAxisVertices * shortAxisVertices;
  const int quadCount = (longAxisVertices - 1)* (shortAxisVertices - 1);
  const int indexCount = quadCount * 6;

  std::vector<VertexPosUvNormTangent> vertices(vertexCount);
  std::vector<IndexEntry> indices(indexCount);

  VertexPosUvNormTangent* vertexTarget = &vertices[0];
  IndexEntry* indexTarget = &indices[0];

  /// Generate vertices
  const float yStep = Pi / float(shortAxisVertices - 1);
  const float xStep = 2.0f * Pi / float(longAxisVertices - 1);
  const float uStep = 1.0f / float(longAxisVertices - 1);
  const float vStep = 1.0f / float(shortAxisVertices - 1);
  float yCoord = 0.0f;
  float v = 0.0f;
  for (int y = 0; y < shortAxisVertices; y++) {
    float xCoord = 0.0f;
    float u = 0.0f;
    for (int x = 0; x < longAxisVertices; x++) {
      const float radius = sinf(yCoord);
      const float yc = cosf(yCoord);
      const float xc = sinf(xCoord) * radius;
      const float zc = cosf(xCoord) * radius;
      vec3 spherical(xc, yc, zc);
      vertexTarget->mPosition = spherical * size;
      vertexTarget->mUv = vec2(u, v);
      vertexTarget->mNormal = spherical;
      vertexTarget->mTangent = normalize(cross(vec3(0, 1, 0), spherical));
      vertexTarget++;
      xCoord += xStep;
      u += uStep;
    }
    yCoord += yStep;
    v += vStep;
  }

  /// Generate indices
  for (int y = 0; y < shortAxisVertices-1; y++) {
    for (int x = 0; x < longAxisVertices-1; x++) {
      const int p = y * longAxisVertices + x;
      indexTarget[0] = p;
      indexTarget[1] = p + 1;
      indexTarget[2] = p + longAxisVertices;
      indexTarget[3] = p + longAxisVertices;
      indexTarget[4] = p + 1;
      indexTarget[5] = p + 1 + longAxisVertices;
      indexTarget += 6;
    }
  }

  mMesh->AllocateVertices(VertexPosUvNormTangent::mFormat, vertexCount);
  mMesh->AllocateIndices(indexCount);

  mMesh->UploadVertices(&vertices[0]);
  mMesh->UploadIndices(&indices[0]);
}

void PolarSphereMeshNode::HandleMessage(Message* message) {
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

