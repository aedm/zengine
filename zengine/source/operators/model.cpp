#include <include/operators/model.h>
#include <include/base/helpers.h>

Model::Model()
	: Node(NodeType::MODEL, string("Model"))
	, TheShader(NodeType::SHADER, this, make_shared<string>("Shader"))
	, TheMesh(this, make_shared<string>("Mesh"))
	, Mapper(NULL)
{
	Slots.push_back(&TheShader);
	Slots.push_back(&TheMesh);
}

/// TODO: Use C++11 delegating constructors
Model::Model( const Model& Original )
	: Node(Original)
	, TheShader(NodeType::SHADER, this, make_shared<string>("Shader"))
	, TheMesh(this, make_shared<string>("Mesh"))
	, Mapper(NULL)
{}

void Model::Render(PrimitiveTypeEnum Primitive)
{
	Evaluate();
	if (!Mapper) return;

	static_cast<ShaderNode*>(TheShader.GetConnectedNode())->Set();
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

void Model::OnSlotConnectionsChanged( Slot* S )
{
	SafeDelete(Mapper);
}

void Model::Operate()
{
	if (Mapper == NULL)
	{
		const Mesh* mesh = TheMesh.GetMesh();
		ShaderNode* shader = (ShaderNode*)TheShader.GetConnectedNode();
		if (mesh && shader)
		{
			Mapper = TheDrawingAPI->CreateAttributeMapper(mesh->Format->Attributes, 
				shader->ShaderProgram->Attributes, mesh->Format->Stride);
		}
	}
}

Model* Model::Create( ShaderNode* ShaderOp, StaticMeshNode* MeshOp )
{
	Model* model = new Model();
	model->TheMesh.Connect(MeshOp);
	model->TheShader.Connect(ShaderOp);
	return model;
}

Node* Model::Clone() const
{
	return new Model(*this);
}
