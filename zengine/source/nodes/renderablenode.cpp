#include <include/nodes/renderablenode.h>
#include <include/base/helpers.h>

RenderableNode::RenderableNode()
	: Node(NodeType::MODEL, string("Model"))
	, TheShader(NodeType::SHADER, this, make_shared<string>("Shader"))
	, TheMesh(this, make_shared<string>("Mesh"))
	, Mapper(nullptr)
{
	Slots.push_back(&TheShader);
	Slots.push_back(&TheMesh);
}

/// TODO: Use C++11 delegating constructors
RenderableNode::RenderableNode( const RenderableNode& Original )
	: Node(Original)
	, TheShader(NodeType::SHADER, this, make_shared<string>("Shader"))
	, TheMesh(this, make_shared<string>("Mesh"))
	, Mapper(nullptr)
	, MappedVertexFormat(nullptr)
{}

void RenderableNode::Render(PrimitiveTypeEnum Primitive)
{
	Evaluate();
	if (!Mapper) return;

	static_cast<ShaderNode*>(TheShader.GetNode())->Set();
	const Mesh* mesh = TheMesh.GetMesh();
	if (mesh->IndexHandle)
	{
		TheDrawingAPI->RenderIndexedMesh(mesh->IndexHandle, mesh->IndexCount, 
			mesh->VertexHandle, Mapper, Primitive);
	} else {
		TheDrawingAPI->RenderMesh(mesh->VertexHandle, mesh->VertexCount, Mapper, 
			Primitive);
	}
}

void RenderableNode::HandleMessage(Slot* S, NodeMessage Message)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		SafeDelete(Mapper);
		MappedVertexFormat = nullptr;
		break;
	case NodeMessage::VALUE_CHANGED:
		if (S == &TheShader || (S == &TheMesh && TheMesh.GetMesh()->Format != MappedVertexFormat))
		{
			SafeDelete(Mapper);
			MappedVertexFormat = nullptr;
		}
		break;
	default:
		break;
	}
}

void RenderableNode::Operate()
{
	if (Mapper == NULL)
	{
		const Mesh* mesh = TheMesh.GetMesh();
		ShaderNode* shader = (ShaderNode*)TheShader.GetNode();
		if (mesh && shader)
		{
			MappedVertexFormat = mesh->Format;
			Mapper = TheDrawingAPI->CreateAttributeMapper(mesh->Format->Attributes, 
				shader->ShaderProgram->Attributes, mesh->Format->Stride);
		}
	}
}

RenderableNode* RenderableNode::Create( ShaderNode* ShaderOp, StaticMeshNode* MeshOp )
{
	RenderableNode* model = new RenderableNode();
	model->TheMesh.Connect(MeshOp);
	model->TheShader.Connect(ShaderOp);
	return model;
}

Node* RenderableNode::Clone() const
{
	return new RenderableNode(*this);
}
