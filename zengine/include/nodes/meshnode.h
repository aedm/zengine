#pragma once

#include "../dom/node.h"
#include "../resources/mesh.h"
#include "../nodes/valuenodes.h"

/// Abstract Mesh node.
class MeshNode: public Node {
public:
  MeshNode();

  Mesh*	GetMesh() const;

protected:
  Mesh*	mMesh;
};

typedef TypedSlot<NodeType::MESH, MeshNode> MeshSlot;


/// A simple static mesh container.
class StaticMeshNode: public MeshNode {
public:
  StaticMeshNode();
  virtual ~StaticMeshNode();

  static StaticMeshNode* Create(OWNERSHIP Mesh* mesh);
};