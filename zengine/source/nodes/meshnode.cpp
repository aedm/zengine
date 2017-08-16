#include <include/nodes/meshnode.h>
#include <include/resources/resourcemanager.h>

REGISTER_NODECLASS(StaticMeshNode, "Static Mesh");

MeshNode::MeshNode()
  : Node(NodeType::MESH) {}


Mesh* MeshNode::GetMesh() const {
  return mMesh;
}


StaticMeshNode::StaticMeshNode()
  : MeshNode() {}


StaticMeshNode::~StaticMeshNode() {
  if (mMesh) TheResourceManager->DiscardMesh(mMesh);
}


StaticMeshNode* StaticMeshNode::Create(OWNERSHIP Mesh* mesh) {
  StaticMeshNode* meshNode = new StaticMeshNode();
  meshNode->mMesh = mesh;
  return meshNode;
}

void StaticMeshNode::Set(OWNERSHIP Mesh* mesh) {
  if (mMesh) TheResourceManager->DiscardMesh(mMesh);
  mMesh = mesh;
  SendMsg(MessageType::VALUE_CHANGED);
}


