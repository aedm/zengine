#include <include/shader/material.h>


Material::Material()
	: Node(NodeType::MATERIAL, "Material")
	, SolidPass(NodeType::PASS, this, make_shared<string>("Solid pass"))
{}


Material::~Material()
{}

void Material::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
}

Pass* Material::GetPass()
{
	return static_cast<Pass*>(SolidPass.GetNode());
}
