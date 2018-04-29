#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

/// TODO: remove this. It's only here because if I put it in a separate, empty .cpp
/// file, the symbol won't get pulled in by the linker, and the nodeclass won't
/// get registered. Fuk C++.
#undef ITEM
#define ITEM(name, capitalizedName, type) \
  REGISTER_NODECLASS(capitalizedName##Node, MAGIC(capitalizedName));
VALUETYPE_LIST


/// Array for attribute types
const ValueType gVertexAttributeType[] = {
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


NodeClass* NodeRegistry::GetNodeClass(const shared_ptr<Node>& node) {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  ASSERT(node.use_count() > 0);
  return mNodeIndexMap.at(type_index(typeid(*node)));
}

