#pragma once

#include "../dom/node.h"
#include "../resources/mesh.h"
#include "../nodes/valuenodes.h"

class MeshSlot: public Slot {
public:
  MeshSlot(Node* owner, SharedString name);

  const Mesh*	GetMesh() const;
};


/// Abstract Mesh node.
class MeshNode: public Node {
public:
  MeshNode();

  Mesh*	GetMesh() const;

protected:
  Mesh*	mMesh;
};


/// A simple static mesh container.
class StaticMeshNode: public MeshNode {
public:
  StaticMeshNode();
  virtual ~StaticMeshNode();

  static StaticMeshNode* Create(OWNERSHIP Mesh* mesh);
};