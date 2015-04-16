#pragma once

#include "../dom/node.h"
#include "../resources/mesh.h"
#include "valueOperators.h"

class MeshOperator;
typedef TypedSlot<NodeType::MESH> MeshSlot;

class MeshOperator : public ValueNode<NodeType::MESH>
{
public:
	MeshOperator();
	virtual ~MeshOperator();

	static MeshOperator*		Create(OWNERSHIP Mesh* Value);

	virtual void				SetValue(Mesh* const & NewValue) override;
};