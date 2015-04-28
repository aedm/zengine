#pragma once

#include "../nodes/shadernode.h"
#include "../nodes/meshnode.h"
#include "../resources/mesh.h"
#include "../render/drawingapi.h"

/// 3D model. Consist of a mesh and a material.
class RenderableNode: public Node
{
public:
	RenderableNode();
	RenderableNode(const RenderableNode& Original);

	void					Render(PrimitiveTypeEnum Primitive = PRIMITIVE_TRIANGLES);

	Slot					TheShader;
	MeshSlot				TheMesh;

	static RenderableNode*			Create(ShaderNode* ShaderOp, StaticMeshNode* MeshOp);

	/// Clone operator
	virtual Node*	Clone() const;

private:
	virtual void			HandleMessage(Slot* S, NodeMessage Message) override;

	virtual void			Operate() override;

	AttributeMapper*		Mapper;
	VertexFormat*			MappedVertexFormat;
};