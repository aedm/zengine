#include <include/nodes/meshnode.h>
#include <include/resources/resourcemanager.h>


MeshNode::MeshNode()
  : Node(NodeType::MESH) {}


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


MeshSlot::MeshSlot(Node* owner, SharedString name)
  : Slot(NodeType::MESH, owner, name) {}


const Mesh* MeshSlot::GetMesh() const {
  MeshNode* node = static_cast<MeshNode*>(GetNode());
  return node == nullptr ? nullptr : node->GetMesh();
}


Mesh* MeshNode::GetMesh() const {
  return mMesh;
}
