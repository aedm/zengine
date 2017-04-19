#define _CRT_SECURE_NO_WARNINGS

#include <include/resources/mesh.h>
#include <include/render/drawingapi.h>
#include <include/base/helpers.h>

VertexFormat* VertexPos::format;
VertexFormat* VertexPosNorm::format;
VertexFormat* VertexPosUVNorm::format;
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
  if (mVertexHandle) TheDrawingAPI->DestroyVertexBuffer(mVertexHandle);
  if (mIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mIndexHandle);
  if (mWireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mWireframeIndexHandle);
  SafeDelete(mRawVertexData);
}

void Mesh::Render(const vector<ShaderAttributeDesc>& usedAttributes, 
                  UINT instanceCount, 
                  PrimitiveTypeEnum primitive) const 
{
    /// Set vertex buffer and attributes
    TheDrawingAPI->SetVertexBuffer(mVertexHandle);
    for (const ShaderAttributeDesc& desc : usedAttributes) {
      VertexAttribute* attribute = mFormat->mAttributesArray[(UINT)desc.Usage];
      if (attribute != nullptr) {
        TheDrawingAPI->EnableVertexAttribute(desc.Handle,
                                             gVertexAttributeType[(UINT)desc.Usage], 
                                             attribute->Offset,
                                             mFormat->mStride);
      } else {
        SHOULD_NOT_HAPPEN;
      }
    }

    if (mIndexHandle) {
      TheDrawingAPI->Render(mIndexHandle, mIndexCount, primitive, instanceCount);
    } else {
      TheDrawingAPI->Render(0, mVertexCount, primitive, instanceCount);
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

    if (mVertexHandle) TheDrawingAPI->DestroyVertexBuffer(mVertexHandle);
    mVertexHandle = TheDrawingAPI->CreateVertexBuffer(newBufferSize);
  }
}

void Mesh::AllocateIndices(UINT indexCount) {
  if (this->mIndexCount != indexCount) {
    this->mIndexCount = indexCount;
    if (mIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mIndexHandle);
    mIndexHandle = indexCount ? TheDrawingAPI->CreateIndexBuffer(indexCount * sizeof(IndexEntry)) : NULL;
  }
}

void Mesh::AllocateWireframeIndices(UINT indexCount) {
  if (mWireframeIndexCount != indexCount) {
    mWireframeIndexCount = indexCount;
    if (mWireframeIndexHandle) TheDrawingAPI->DestroyIndexBuffer(mWireframeIndexHandle);
    mWireframeIndexHandle = indexCount ? TheDrawingAPI->CreateIndexBuffer(mWireframeIndexCount) : NULL;
  }
}

void Mesh::UploadIndices(const IndexEntry* indices) {
  //TheDrawingAPI->UploadIndices(IndexHandle, IndexCount, Indices);
  void* mappedIndices = TheDrawingAPI->MapIndexBuffer(mIndexHandle);
  memcpy(mappedIndices, indices, mIndexCount * sizeof(IndexEntry));
  TheDrawingAPI->UnMapIndexBuffer(mIndexHandle);
}

void Mesh::UploadVertices(void* vertices) {
  void* mappedMesh = TheDrawingAPI->MapVertexBuffer(mVertexHandle);
  memcpy(mappedMesh, vertices, mVertexCount * mFormat->mStride);
  TheDrawingAPI->UnMapVertexBuffer(mVertexHandle);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, mVertexCount * mFormat->mStride);
}


void Mesh::UploadVertices(void* vertices, int vertexCount) {
  void* mappedMesh = TheDrawingAPI->MapVertexBuffer(mVertexHandle);
  memcpy(mappedMesh, vertices, vertexCount * mFormat->mStride);
  TheDrawingAPI->UnMapVertexBuffer(mVertexHandle);

  /// TODO: use unique_ptr or OWNERSHIP instead of copying twice
  memcpy(mRawVertexData, vertices, vertexCount * mFormat->mStride);
}
