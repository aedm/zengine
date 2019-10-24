#include <include/dom/nodetype.h>
#include <include/nodes/valuenodes.h>

NodeRegistry* NodeRegistry::sInstance = nullptr;

NodeRegistry* NodeRegistry::GetInstance() {
  if (sInstance == nullptr) {
    sInstance = new NodeRegistry();
  }
  return sInstance;
}


NodeClass* NodeRegistry::GetNodeClass(const std::string& name) {
  const auto it = mNodeClassesByName.find(name);
  return it == mNodeClassesByName.end() ? nullptr : it->second;
}


NodeClass* NodeRegistry::GetNodeClass(const std::shared_ptr<Node>& node) {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  ASSERT(node.use_count() > 0);
  auto& nodeRef = *node;
  return mNodeIndexMap.at(std::type_index(typeid(nodeRef)));
}

