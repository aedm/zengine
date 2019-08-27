#include <include/nodes/buffernode.h>

REGISTER_NODECLASS(MeshToVertexBufferNode, "Mesh to vertex buffer");

MeshToVertexBufferNode::MeshToVertexBufferNode()
  : mMeshSlot(this, "Mesh")
{}

const std::shared_ptr<Buffer> MeshToVertexBufferNode::GetBuffer() {
  shared_ptr<MeshNode>& meshNode = mMeshSlot.GetNode();
  if (!meshNode) return nullptr;
  auto& mesh = meshNode->GetMesh();
  if (!mesh) return nullptr;
  return mesh->mVertexBuffer;
}

