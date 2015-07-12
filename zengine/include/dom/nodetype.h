#pragma once

#include "../base/types.h"
#include <string>
#include <unordered_map>
#include <typeindex>

using namespace std;
class Node;
class Slot;

/// Possible node types
enum class NodeType {
  /// Node types holding a value type
#undef ITEM
#define ITEM(name, type) name,
  VALUETYPE_LIST

  /// Other node types 
  TEXTURE,
  SAMPLER,
  SHADER_SOURCE,
  SHADER_STUB,
  PASS,
  MATERIAL,
  MODEL,
  MESH,
  DRAWABLE,
  GRAPH,
  DOCUMENT,

  /// Empty stub value type
  NONE,

  /// Editor nodes
  UI,
  WIDGET,

  /// Slot type that allows all node connections
  ALLOW_ALL,

  /// Undefined behavior
  UNDEFINED
};


/// Type helpers
template<NodeType T> struct NodeTypes;
#undef ITEM
#define ITEM(name, type) \
  template<> struct NodeTypes<NodeType::name> { typedef type Type; };
VALUETYPE_LIST


/// Array for attribute types, index by VertexAttributeEnum
extern const NodeType gVertexAttributeType[];


/// Returns true if the node is an instance of a certain class
template<class T>
bool IsInstanceOf(Node* node) {
  return typeid(T) == typeid(*node);
}

template<class T>
bool IsInstanceOf(Slot* slot) {
  return typeid(T) == typeid(*slot);
}


/// Exact type of the node. Poor man's reflection.
struct NodeClass {
  virtual Node* Manufacture() = 0;
  virtual Node* Manufacture(Node*) = 0;
  string mClassName;
};


/// Use this in .cpp files, not in headers!
/// Registers a Node type in the engine. This makes the engine know about
/// all the Node types that can be utilised, and it also enables serialization
/// and deserialization of objects;
#define REGISTER_NODECLASS(nodeClass, nodeClassName)          \
  struct NodeClass_##nodeClass: public NodeClass {            \
    NodeClass_##nodeClass() {                                 \
      mClassName = string(nodeClassName);                     \
      NodeRegistry::GetInstance()->Register<nodeClass>(this); \
    }                                                         \
    virtual Node* Manufacture() override {                    \
      return new nodeClass();                                 \
    }                                                         \
    virtual Node* Manufacture(Node* node) override {          \
      return new nodeClass(*static_cast<nodeClass*>(node));   \
    }                                                         \
  } NodeClassInstance_##nodeClass;                            \


/// Poor man's reflection. This class keeps track of all possilbe Node classes.
class NodeRegistry {
public:
  static NodeRegistry* GetInstance();

  template<class T> void Register(NodeClass* nodeClass);

  /// Returns NodeClass by name, node name doesn't have to be registered
  NodeClass* GetNodeClass(const string& name);

  /// Returns NodeClass by node instance. Node must be registered
  NodeClass* GetNodeClass(Node* node);

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

