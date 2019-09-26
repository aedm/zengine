#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

NodeRegistry* NodeRegistry::sInstance = nullptr;

NodeRegistry* NodeRegistry::GetInstance() {
  if (sInstance == nullptr) {
    sInstance = new NodeRegistry();
  }
  return sInstance;
}


NodeClass* NodeRegistry::GetNodeClass(const string& name) {
  const auto it = mNodeClassesByName.find(name);
  return it == mNodeClassesByName.end() ? nullptr : it->second;
}


NodeClass* NodeRegistry::GetNodeClass(const shared_ptr<Node>& node) {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  ASSERT(node.use_count() > 0);
  return mNodeIndexMap.at(type_index(typeid(node.get())));
}

