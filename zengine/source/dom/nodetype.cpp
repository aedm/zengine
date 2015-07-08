#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(FloatNode);
REGISTER_NODECLASS(Vec4Node);
REGISTER_NODECLASS(Matrix4Node);
REGISTER_NODECLASS(TextureNode);


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

