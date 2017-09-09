#define _CRT_SECURE_NO_WARNINGS

#include <include/resources/mesh.h>
#include <include/render/drawingapi.h>
#include <include/base/helpers.h>

VertexFormat* VertexPos::format;
VertexFormat* VertexPosNorm::format;
VertexFormat* VertexPosUVNorm::format;
VertexFormat* VertexPosUVNormTangent::format;
VertexFormat* VertexPosUV::format;

VertexFormat::VertexFormat(UINT binaryFormat) {
  memset(mAttributesArray, 0, sizeof(void*) * (UINT)VertexAttributeUsage::COUNT);

  this->mBinaryFormat = binaryFormat;
  int stride = 0;
  for (int i = 0; binaryFormat; binaryFormat >>= 1, i++) {
    if (binaryFormat & 1) {
      VertexAttribute attrib;
      attrib.Usage = (VertexAttributeUsage)i;
      attrib.Size = gVariableByteSizes[(UINT)gVertexAttributeType[i]];
      attrib.Offset = stride;
      mAttributes.push_back(attrib);
      stride += attrib.Size;
    }
  }

  for (UINT i = 0; i < mAttributes.size(); i++) {
    VertexAttribute& attrib = mAttributes[i];

    /// AttributesArray points into the vector. Meh.
    mAttributesArray[(UINT)attrib.Usage] = &attrib;
  }

  mStride = stride;
  ASSERT(stride % 4 == 0);	/// Vertex structure size should always be mod4.
}


VertexFormat::~VertexFormat() {}


bool VertexFormat::HasAttribute(VertexAttributeUsage attrib) {
  return (mBinaryFormat & (1 << (UINT)attrib)) != 0;
}


Mesh::Mesh() {
  mVertexCount = 0;
  mVertexBufferSize = 0;
  mVertexHandle = 0;

  mIndexCount = 0;
  mIndexHandle = 0;

  mWireframeIndexCount = 0;
  mWireframeIndexHandle = 0;

  mFormat = nullptr;
}

Mesh::~Mesh() {
  if (mVertexHandle) OpenGL->DestroyVertexBuffer(mVertexHandle);
  if (mIndexHandle) OpenGL->DestroyIndexBuffer(mIndexHandle);
  if (mWireframeIndexHandle) OpenGL->DestroyIndexBuffer(mWireframeIndexHandle);
  SafeDelete(mRawVertexData);
}

void Mesh::Render(const vector<ShaderAttributeDesc>& usedAttributes, 
                  UINT instanceCount, 
                  PrimitiveTypeEnum primitive) const 
{
  /// Set vertex buffer and attributes
  OpenGL->SetVertexBuffer(mVertexHandle);
    for (const ShaderAttributeDesc& desc : usedAttributes) {
      VertexAttribute* attribute = mFormat->mAttributesArray[(UINT)desc.Usage];
      if (attribute != nullptr) {
        OpenGL->EnableVertexAttribute(desc.Handle,
                                             gVertexAttributeType[(UINT)desc.Usage], 
                                             attribute->Offset,
                                             mFormat->mStride);
      } else {
        SHOULD_NOT_HAPPEN;
      }
    }

    if (mIndexHandle) {
      OpenGL->Render(mIndexHandle, mIndexCount, primitive, instanceCount);
    } else {
      OpenGL->Render(0, mVertexCount, primitive, instanceCount);
    }
}

void Mesh::AllocateVertices(VertexFormat* format, UINT vertexCount) {
  this->mFormat = format;
  this->mVertexCount = vertexCount;
  UINT newBufferSize = format->mStride * vertexCount;

  if (mVertexBufferSize != newBufferSize) {
    mVertexBufferSize = newBufferSize;

    SafeDelete(mRawVertexData);
    mRawVertexData = new char[newBufferSize];

    if (mVertexHandle) OpenGL->DestroyVertexBuffer(mVertexHandle);
    mVertexHandle = OpenGL->CreateVertexBuffer(newBufferSize);
  }
}

void Mesh::AllocateIndices(UINT indexCount) {
  if (this->mIndexCount != indexCount) {
    this->mIndexCount = indexCount;
    if (mIndexHandle) OpenGL->DestroyIndexBuffer(mIndexHandle);
    mIndexHandle = indexCount ? OpenGL->CreateIndexBuffer(indexCount * sizeof(IndexEntry)) : NULL;
  }
}

void Mesh::AllocateWireframeIndices(UINT indexCount) {
  if (mWireframeIndexCount != indexCount) {
    mWireframeIndexCount = indexCount;
    if (mWireframeIndexHandle) OpenGL->DestroyIndexBuffer(mWireframeIndexHandle);
    mWireframeIndexHandle = indexCount ? OpenGL->CreateIndexBuffer(mWireframeIndexCount) : NULL;
  }
}

void Mesh::UploadIndices(const IndexEntry* indices) {
  //TheDrawingAPI->UploadIndices(IndexHandle, IndexCount, Indices);
  void* mappedIndices = OpenGL->MapIndexBuffer(mIndexHandle);
  memcpy(mappedIndices, indices, mIndexCount * sizeof(IndexEntry));
  OpenGL->UnMapIndexBuffer(mIndexHandle);

  mIndexData.resize(mIndexCount);
  memcpy(&mIndexData[0], indices, mIndexCount * sizeof(IndexEntry));
}

void Mesh::UploadVertices(void* vertices) {
  void* mappedMesh = OpenGL->MapVertexBuffer(mVertexHandle);
  memcpy(mappedMesh, vertices, mVertexCount * mFormat->mStride);
  OpenGL->UnMapVertexBuffer(mVertexHandle);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, mVertexCount * mFormat->mStride);
}


void Mesh::UploadVertices(void* vertices, int vertexCount) {
  void* mappedMesh = OpenGL->MapVertexBuffer(mVertexHandle);
  memcpy(mappedMesh, vertices, vertexCount * mFormat->mStride);
  OpenGL->UnMapVertexBuffer(mVertexHandle);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, vertexCount * mFormat->mStride);
}
