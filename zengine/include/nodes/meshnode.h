#pragma once

#include "../dom/node.h"
#include "../resources/mesh.h"
#include "../nodes/valuenodes.h"

/// Abstract Mesh node.
class MeshNode: public Node {
public:
  MeshNode();

  const shared_ptr<Mesh>& GetMesh() const;

protected:
  shared_ptr<Mesh> mMesh;
};

typedef TypedSlot<MeshNode> MeshSlot;


/// A simple static mesh container.
class StaticMeshNode: public MeshNode {
public:
  StaticMeshNode();
  void Set(const shared_ptr<Mesh>& mesh);
};

