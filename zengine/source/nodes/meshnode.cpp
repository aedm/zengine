#include <include/nodes/meshnode.h>

REGISTER_NODECLASS(StaticMeshNode, "Static Mesh");

MeshNode::MeshNode() = default;

const std::shared_ptr<Mesh>& MeshNode::GetMesh() const {
  return mMesh;
}

StaticMeshNode::StaticMeshNode()
  : MeshNode()
{}

void StaticMeshNode::Set(const std::shared_ptr<Mesh>& mesh) {
  mMesh = mesh;
  SendMsg(MessageType::VALUE_CHANGED);
}


