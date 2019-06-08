#pragma once

#include "../base/types.h"
#include <string>
#include <unordered_map>
#include <typeindex>

using namespace std;
class Node;
class Slot;


/// Returns true if the node is an instance of a certain class
template<class T>
bool IsExactType(Node* node) {
  return typeid(T) == typeid(*node);
}

template<class T>
bool IsExactType(Slot* slot) {
  return typeid(T) == typeid(*slot);
}

template<class T>
bool IsExactType(const shared_ptr<Node>& node) {
  return typeid(T) == typeid(*node.get());
}

template<class T, class N>
bool IsInsanceOf(N* ptr) {
  return dynamic_cast<T>(ptr) != nullptr;
}

template<class T, class N>
bool IsPointerOf(const shared_ptr<N>& ptr) {
  return dynamic_pointer_cast<T>(ptr) != nullptr;
}


/// Exact type of the node. Poor man's reflection.
struct NodeClass {
  virtual shared_ptr<Node> Manufacture() = 0;
  string mClassName;
};


/// Poor man's reflection. This class keeps track of all possilbe Node classes.
class NodeRegistry {
public:
  static NodeRegistry* GetInstance();

  template<class T> void Register(NodeClass* nodeClass);

  /// Returns NodeClass by name, node name doesn't have to be registered
  NodeClass* GetNodeClass(const string& name);

  /// Returns NodeClass by node instance. Node must be registered
  NodeClass* GetNodeClass(const shared_ptr<Node>& node);

  /// Returns NodeClass by node class. Node must be registered
  template<class T> NodeClass* GetNodeClass();

private:
  //NodeRegistry();
  static NodeRegistry* sInstance;

  unordered_map<string, NodeClass*> mNodeClassesByName;
  unordered_map<type_index, NodeClass*> mNodeIndexMap;
};

template<class T>
void NodeRegistry::Register(NodeClass* nodeClass) {
  mNodeClassesByName[nodeClass->mClassName] = nodeClass;
  mNodeIndexMap[type_index(typeid(T))] = nodeClass;
}

template<class T>
NodeClass* NodeRegistry::GetNodeClass() {
  /// If this breaks, you forgot to REGISTER_NODECLASS.
  return mNodeIndexMap.at(type_index(typeid(T)));
}


/// Use this in .cpp files, not in headers!
/// Registers a Node type in the engine. This makes the engine know about
/// all the Node types that can be utilised, and it also enables serialization
/// and deserialization of objects;
#define REGISTER_NODECLASS(nodeClass, nodeClassName)                               \
  struct NodeClass_##nodeClass: public NodeClass {                                 \
    NodeClass_##nodeClass() {                                                      \
      mClassName = string(nodeClassName);                                          \
      NodeRegistry::GetInstance()->Register<nodeClass>(this);					   \
    }                                                                              \
    virtual shared_ptr<Node> Manufacture() override {                              \
      return make_shared<nodeClass>();                                             \
    }                                                                              \
  } NodeClassInstance_##nodeClass;                                                 \
//__pragma(comment(linker, "/include:" #nodeClass));
