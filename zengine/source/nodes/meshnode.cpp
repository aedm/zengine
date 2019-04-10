#include <include/nodes/meshnode.h>
#include <include/resources/resourcemanager.h>

REGISTER_NODECLASS(StaticMeshNode, "Static Mesh");

MeshNode::MeshNode() {}

const shared_ptr<Mesh>& MeshNode::GetMesh() const {
  return mMesh;
}

StaticMeshNode::StaticMeshNode()
  : MeshNode()
{}

void StaticMeshNode::Set(const shared_ptr<Mesh>& mesh) {
  mMesh = mesh;
  SendMsg(MessageType::VALUE_CHANGED);
}


