#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

class Node;
class Slot;

/// Returns true if the node is an instance of a certain class
template<class T>
// ReSharper disable once CppEntityUsedOnlyInUnevaluatedContext
bool IsExactType(Node* node) {
  return typeid(T) == typeid(*node);
}

template<class T>
// ReSharper disable once CppEntityUsedOnlyInUnevaluatedContext
bool IsExactType(Slot* slot) {
  return typeid(T) == typeid(*slot);
}

template<class T>
// ReSharper disable once CppEntityUsedOnlyInUnevaluatedContext
bool IsExactType(const std::shared_ptr<Node>& node) {
  return typeid(T) == typeid(*node.get());
}

template<class T, class N>
bool IsInstanceOf(N* ptr) {
  return dynamic_cast<T>(ptr) != nullptr;
}

template<class T, class N>
bool IsPointerOf(const std::shared_ptr<N>& ptr) {
  return std::dynamic_pointer_cast<T>(ptr) != nullptr;
}


/// Exact type of the node. Poor man's reflection.
struct NodeClass {
  virtual ~NodeClass() = default;
  virtual std::shared_ptr<Node> Manufacture() = 0;
  std::string mClassName;
};


/// Poor man's reflection. This class keeps track of all possible Node classes.
class NodeRegistry {
public:
  static NodeRegistry* GetInstance();

  template<class T> void Register(NodeClass* nodeClass);

  /// Returns NodeClass by name, node name does not have to be registered
  NodeClass* GetNodeClass(const std::string& name);

  /// Returns NodeClass by node instance. Node must be registered
  NodeClass* GetNodeClass(const std::shared_ptr<Node>& node);

  /// Returns NodeClass by node class. Node must be registered
  template<class T> NodeClass* GetNodeClass();

private:
  //NodeRegistry();
  static NodeRegistry* sInstance;

  std::unordered_map<std::string, NodeClass*> mNodeClassesByName;
  std::unordered_map<std::type_index, NodeClass*> mNodeIndexMap;
};

template<class T>
void NodeRegistry::Register(NodeClass* nodeClass) {
  mNodeClassesByName[nodeClass->mClassName] = nodeClass;
  mNodeIndexMap[std::type_index(typeid(T))] = nodeClass;
}

template<class T>
NodeClass* NodeRegistry::GetNodeClass() {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  return mNodeIndexMap.at(std::type_index(typeid(T)));
}


/// Use this in .cpp files, not in headers!
/// Registers a Node type in the engine. This makes the engine know about
/// all the Node types that can be utilized, and it also enables serialization
/// and deserialization of objects;
#define REGISTER_NODECLASS(nodeClass, nodeClassName)                               \
  struct NodeClass_##nodeClass: public NodeClass {                                 \
    NodeClass_##nodeClass() {                                                      \
      mClassName = std::string(nodeClassName);                                          \
      NodeRegistry::GetInstance()->Register<nodeClass>(this);					   \
    }                                                                              \
    virtual std::shared_ptr<Node> Manufacture() override {                              \
      return std::make_shared<nodeClass>();                                             \
    }                                                                              \
  } NodeClassInstance_##nodeClass;                                                 \
//__pragma(comment(linker, "/include:" #nodeClass));
