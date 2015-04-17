#pragma once

#include "../nodes/shadernode.h"
#include "../nodes/staticmeshnode.h"
#include "../resources/mesh.h"
#include "../render/drawingapi.h"

/// 3D model. Consist of a mesh and a material.
class Model: public Node
{
public:
	Model();
	Model(const Model& Original);

	void					Render(PrimitiveTypeEnum Primitive = PRIMITIVE_TRIANGLES);

	Slot					TheShader;
	MeshSlot				TheMesh;

	static Model*			Create(ShaderNode* ShaderOp, StaticMeshNode* MeshOp);

	/// Clone operator
	virtual Node*	Clone() const;

private:
	virtual void			OnSlotConnectionsChanged(Slot* S) override;
	virtual void			Operate() override;

	AttributeMapper*		Mapper;
};