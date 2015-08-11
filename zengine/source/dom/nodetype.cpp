#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

/// TODO: remove this. It's only here because if I put it in a separate, empty .cpp
/// file, the symbol won't get pulled in by the linker, and the nodeclass won't
/// get registered. Fuk C++.
#include <include/resources/texture.h>
REGISTER_NODECLASS(TextureNode, "Texture");

REGISTER_NODECLASS(StringNode, "String");
REGISTER_NODECLASS(FloatNode, "Float");
REGISTER_NODECLASS(Vec4Node, "Vec4");
REGISTER_NODECLASS(Vec3Node, "Vec3");
REGISTER_NODECLASS(Matrix4Node, "Matrix4");


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

