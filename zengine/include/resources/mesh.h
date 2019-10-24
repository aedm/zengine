#pragma once

#include "../base/defines.h"
#include "../shaders/valuetype.h"
#include "../render/drawingapi.h"
#include <vector>
#include <memory>
#include <memory>

class Mesh {
public:
  ~Mesh();

  void Render(UINT instanceCount, PrimitiveTypeEnum primitive) const;

  void AllocateVertices(const std::shared_ptr<VertexFormat>& format, UINT vertexCount);
  void AllocateIndices(UINT indexCount);

  /// Uploads all vertices
  void UploadVertices(void* vertices) const;

  /// Uploads only the first VertexCount vertices, doessn't reallocate
  void UploadVertices(void* vertices, int vertexCount) const;

  /// Uploads all indices
  void UploadIndices(const IndexEntry* indices);

  template<typename T, int N>	void SetVertices(const T(&staticVertices)[N]);
  template<int N> void SetIndices(const IndexEntry(&staticIndices)[N]);

  UINT mVertexCount = 0;
  const std::shared_ptr<Buffer> mVertexBuffer = std::make_shared<Buffer>();

  UINT mIndexCount = 0;
  const std::shared_ptr<Buffer> mIndexBuffer = std::make_shared<Buffer>();

  std::shared_ptr<VertexFormat> mFormat = nullptr;

  /// Raw mesh data for deserialization
  void* mRawVertexData = nullptr;
  std::vector<IndexEntry> mIndexData;
};

template<typename T, int N>
void Mesh::SetVertices(const T(&staticVertices)[N]) {
  AllocateVertices(T::mFormat, N);
  // ReSharper disable once CppCStyleCast
  UploadVertices((void*)staticVertices);
  SafeDelete(mRawVertexData);
  mRawVertexData = new char[N * T::mFormat->mStride];
}

template<int N>
void Mesh::SetIndices(const IndexEntry(&staticIndices)[N]) {
  AllocateIndices(N);
  UploadIndices(staticIndices);
}
