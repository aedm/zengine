#include <include/shaders/material.h>


Material::Material()
  : Node(NodeType::MATERIAL, "Material")
  , mSolidPass(NodeType::PASS, this, make_shared<string>("Solid pass")) {}


Material::~Material() {}

void Material::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {}

Pass* Material::GetPass() {
  return static_cast<Pass*>(mSolidPass.GetNode());
}
