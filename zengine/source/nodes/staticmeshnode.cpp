#include <include/nodes/staticmeshnode.h>
#include <include/resources/resourcemanager.h>


MeshNode::MeshNode()
	: Node(NodeType::MESH, "Meshzzz")
{}


StaticMeshNode::StaticMeshNode()
	: MeshNode()
{}

StaticMeshNode::~StaticMeshNode()
{
	if (MeshValue) TheResourceManager->DiscardMesh(MeshValue);
}

StaticMeshNode* StaticMeshNode::Create( OWNERSHIP Mesh* _Value )
{
	StaticMeshNode* meshNode = new StaticMeshNode();
	meshNode->MeshValue = _Value;
	return meshNode;
}

MeshSlot::MeshSlot(Node* Owner, SharedString Name)
	: Slot(NodeType::MESH, Owner, Name)
{}

const Mesh* MeshSlot::GetMesh() const
{
	MeshNode* node = static_cast<MeshNode*>(GetConnectedNode());
	return node == nullptr ? nullptr : node->GetMesh();
}


Mesh* MeshNode::GetMesh() const
{
	return MeshValue;
}
