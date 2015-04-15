#pragma once

#include "../dom/node.h"
#include "../resources/mesh.h"
#include "valueOperators.h"

class MeshOperator;
typedef TypedSlot<NODE_MESH> MeshSlot;

class MeshOperator: public ValueOperator<NODE_MESH>
{
public:
	MeshOperator();
	virtual ~MeshOperator();

	static MeshOperator*		Create(OWNERSHIP Mesh* Value);

	virtual void				SetValue(Mesh* const & NewValue) override;
};