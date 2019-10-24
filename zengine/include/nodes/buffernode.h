#pragma once

#include "../render/drawingapi.h"
#include "../dom/node.h"
#include "meshnode.h"

class BufferNode : public Node {
public:
  virtual std::shared_ptr<Buffer> GetBuffer() = 0;
  
protected:
  BufferNode() = default;
};

typedef TypedSlot<BufferNode> BufferSlot;


class MeshToVertexBufferNode : public BufferNode {
public:
  MeshToVertexBufferNode();
  std::shared_ptr<Buffer> GetBuffer() override;

  MeshSlot mMeshSlot;
};


