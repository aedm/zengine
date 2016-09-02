#pragma once

#include "../base/defines.h"
#include "../base/vectormath.h"
#include "../render/drawingapi.h"
#include <vector>

using namespace std;

class ResourceManager;
class VertexFormat;

enum VertexAttributeMask {
  VERTEXATTRIB_POSITION_MASK = 1 << (UINT)VertexAttributeUsage::POSITION,
  VERTEXATTRIB_TEXCOORD_MASK = 1 << (UINT)VertexAttributeUsage::TEXCOORD,
  VERTEXATTRIB_NORMAL_MASK = 1 << (UINT)VertexAttributeUsage::NORMAL,
  VERTEXATTRIB_BINORMAL_MASK = 1 << (UINT)VertexAttributeUsage::BINORMAL,
  //VERTEXATTRIB_COLOR_MASK			= 1 << VERTEXATTRIB_COLOR
};


/// Common vertex formats
struct VertexPos {
  Vec3 position;

  /// Vertex format descriptor for this struct
  static VertexFormat* format;
};

struct VertexPosNorm {
  Vec3 position;
  Vec3 normal;

  /// Vertex format descriptor for this struct
  static VertexFormat* format;
};

struct VertexPosUVNorm {
  Vec3 position;
  Vec2 uv;
  Vec3 normal;

  /// Vertex format descriptor for this struct
  static VertexFormat* format;
};

struct VertexPosUV {
  Vec3 position;
  Vec2 uv;

  /// Vertex format descriptor for this struct
  static VertexFormat* format;
};


class VertexFormat {
  friend class ResourceManager;

  VertexFormat(UINT binaryFormat);
  ~VertexFormat();

public:
  void Set();
  //void SetAttributeDefines(ShaderDefines& Defines);

  bool HasAttribute(VertexAttributeUsage attrib);

  //VertexDeclaration			Declaration;
  int mStride;
  UINT mBinaryFormat;
  vector<VertexAttribute> mAttributes;

  VertexAttribute* mAttributesArray[(UINT)VertexAttributeUsage::COUNT];
};


class Mesh {
  friend class ResourceManager;

  Mesh();
  ~Mesh();

public:
  void Render(const vector<ShaderAttributeDesc>& usedAttributes,
              UINT instanceCount,
              PrimitiveTypeEnum primitive) const;

  void AllocateVertices(VertexFormat* format, UINT vertexCount);
  void AllocateIndices(UINT indexCount);
  void AllocateWireframeIndices(UINT indexCount);

  /// Uploads all vertices
  void UploadVertices(void* vertices);

  /// Uploads only the first VertexCount vertices, doessn't reallocate
  void UploadVertices(void* vertices, int vertexCount);

  /// Uploads all indices
  void UploadIndices(const IndexEntry* indices);

  template<typename T, int N>	void SetVertices(const T(&staticVertices)[N]);
  template<int N> void SetIndices(const IndexEntry(&staticIndices)[N]);

  UINT mVertexCount;
  UINT mVertexBufferSize;
  VertexBufferHandle mVertexHandle;

  UINT mIndexCount;
  IndexBufferHandle mIndexHandle;

  UINT mWireframeIndexCount;
  IndexBufferHandle	mWireframeIndexHandle;

  VertexFormat* mFormat;

  /// Raw vertex data for deserialization
  void* mRawVertexData = nullptr;
};

template<typename T, int N>
void Mesh::SetVertices(const T(&staticVertices)[N]) {
  AllocateVertices(T::format, N);
  UploadVertices((void*)staticVertices);

  SafeDelete(mRawVertexData);
  mRawVertexData = new char[N * T::format->mStride];
}

template<int N>
void Mesh::SetIndices(const IndexEntry(&staticIndices)[N]) {
  AllocateIndices(N);
  UploadIndices(staticIndices);
}
