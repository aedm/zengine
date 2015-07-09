#include <include/nodes/meshnode.h>
#include <include/resources/resourcemanager.h>


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


