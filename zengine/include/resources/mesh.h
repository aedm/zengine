#pragma once

#include "../base/defines.h"
#include "../base/vectormath.h"
#include "../shaders/valuetype.h"
#include "../render/drawingapi.h"
#include <vector>

using namespace std;
class Mesh {
public:
  //Mesh();
  ~Mesh();

  void Render(UINT instanceCount, PrimitiveTypeEnum primitive) const;

  void AllocateVertices(const shared_ptr<VertexFormat>& format, UINT vertexCount);
  void AllocateIndices(UINT indexCount);
  //void AllocateWireframeIndices(UINT indexCount);

  /// Uploads all vertices
  void UploadVertices(void* vertices) const;

  /// Uploads only the first VertexCount vertices, doessn't reallocate
  void UploadVertices(void* vertices, int vertexCount) const;

  /// Uploads all indices
  void UploadIndices(const IndexEntry* indices);

  template<typename T, int N>	void SetVertices(const T(&staticVertices)[N]);
  template<int N> void SetIndices(const IndexEntry(&staticIndices)[N]);

  UINT mVertexCount = 0;
  const shared_ptr<Buffer> mVertexBuffer = make_shared<Buffer>();

  UINT mIndexCount = 0;
  const shared_ptr<Buffer> mIndexBuffer = make_shared<Buffer>();

  shared_ptr<VertexFormat> mFormat = nullptr;

  /// Raw mesh data for deserialization
  void* mRawVertexData = nullptr;
  vector<IndexEntry> mIndexData;
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
