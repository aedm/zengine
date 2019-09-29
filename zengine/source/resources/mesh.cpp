#define _CRT_SECURE_NO_WARNINGS

#include <include/resources/mesh.h>
#include <include/render/drawingapi.h>
#include <include/base/helpers.h>

shared_ptr<VertexFormat> VertexPos::mFormat = make_shared<VertexFormat>(
  VERTEXATTRIB_POSITION_MASK);
shared_ptr<VertexFormat> VertexPosNorm::mFormat = make_shared<VertexFormat>(
  VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_NORMAL_MASK);
shared_ptr<VertexFormat> VertexPosUvNorm::mFormat = make_shared<VertexFormat>(
  VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_NORMAL_MASK | VERTEXATTRIB_TEXCOORD_MASK);
shared_ptr<VertexFormat> VertexPosUvNormTangent::mFormat = make_shared<VertexFormat>(
  VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_NORMAL_MASK | VERTEXATTRIB_TEXCOORD_MASK |
  VERTEXATTRIB_TANGENT_MASK);
shared_ptr<VertexFormat> VertexPosUv::mFormat = make_shared<VertexFormat>(
  VERTEXATTRIB_POSITION_MASK | VERTEXATTRIB_TEXCOORD_MASK);


VertexFormat::VertexFormat(UINT binaryFormat) {
  memset(mAttributesArray, 0, sizeof(void*) * UINT(VertexAttributeUsage::COUNT));

  this->mBinaryFormat = binaryFormat;
  int stride = 0;
  for (int i = 0; binaryFormat; binaryFormat >>= 1, i++) {
    if (binaryFormat & 1) {
      VertexAttribute attribute{};
      attribute.Usage = VertexAttributeUsage(i);
      attribute.Size = ValueTypeByteSize(VertexAttributeUsageToValueType(attribute.Usage));
      attribute.Offset = stride;
      mAttributes.push_back(attribute);
      stride += attribute.Size;
    }
  }

  for (auto& attribute : mAttributes)
  {
    /// AttributesArray points into the vector. Meh.
    mAttributesArray[UINT(attribute.Usage)] = &attribute;
  }

  mStride = stride;
  ASSERT(stride % 4 == 0);	/// Vertex structure size should always be mod4.
}

Mesh::~Mesh() {
  SafeDelete(mRawVertexData);
}

void Mesh::Render(//const vector<ShaderProgram::Attribute>& usedAttributes,
  UINT instanceCount,
  PrimitiveTypeEnum primitive) const {
  /// Set vertex buffer and attributes
  OpenGL->SetVertexBuffer(mVertexBuffer);

  /// Bind all attributes to their fixed layout location
  for (auto& attribute : mFormat->mAttributes) {
    OpenGLAPI::EnableVertexAttribute(UINT(attribute.Usage),
      VertexAttributeUsageToValueType(attribute.Usage),
      attribute.Offset, mFormat->mStride);
  }

  if (mIndexBuffer->IsEmpty()) {
    /// Render all vertices without index buffer
    OpenGL->Render(nullptr, mVertexCount, primitive, instanceCount);
  }
  else {
    /// Render indexed mesh
    OpenGL->Render(mIndexBuffer, mIndexCount, primitive, instanceCount);
  }
}

void Mesh::AllocateVertices(const shared_ptr<VertexFormat>& format, UINT vertexCount) {
  this->mFormat = format;
  this->mVertexCount = vertexCount;
  const int newBufferSize = format->mStride * vertexCount;

  if (mVertexBuffer->GetByteSize() != newBufferSize) {
    mVertexBuffer->Allocate(newBufferSize);
    SafeDelete(mRawVertexData);
    mRawVertexData = new char[newBufferSize];
  }
}

void Mesh::AllocateIndices(UINT indexCount) {
  if (mIndexCount != indexCount) {
    mIndexCount = indexCount;
    mIndexBuffer->Allocate(indexCount * sizeof(IndexEntry));
  }
}

void Mesh::UploadIndices(const IndexEntry* indices) {
  mIndexBuffer->UploadData(indices, mIndexCount * sizeof(IndexEntry));
  mIndexData.resize(mIndexCount);
  memcpy(&mIndexData[0], indices, mIndexCount * sizeof(IndexEntry));
}

void Mesh::UploadVertices(void* vertices) const
{
  mVertexBuffer->UploadData(vertices, mVertexCount * mFormat->mStride);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, mVertexCount * mFormat->mStride);
}


void Mesh::UploadVertices(void* vertices, int vertexCount) const
{
  mVertexBuffer->UploadData(vertices, vertexCount * mFormat->mStride);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, vertexCount * mFormat->mStride);
}
