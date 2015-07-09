#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(FloatNode, "Float");
REGISTER_NODECLASS(Vec4Node, "Vec4");
REGISTER_NODECLASS(Matrix4Node, "Matrix4");
REGISTER_NODECLASS(TextureNode, "Texture");


/// Array for attribute types
const NodeType gVertexAttributeType[] = {
#undef ITEM
#define ITEM(name, type, token) type,
  VERTEXATTRIBUTE_LIST
};

NodeRegistry* NodeRegistry::sInstance = nullptr;

NodeRegistry* NodeRegistry::GetInstance() {
  if (sInstance == nullptr) {
    sInstance = new NodeRegistry();
  }
  return sInstance;
}


NodeClass* NodeRegistry::GetNodeClass(const string& name) {
  auto it = mNodeClassesByName.find(name);
  return it == mNodeClassesByName.end() ? nullptr : it->second;
}


NodeClass* NodeRegistry::GetNodeClass(Node* node) {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  return mNodeIndexMap.at(type_index(typeid(*node)));
}

