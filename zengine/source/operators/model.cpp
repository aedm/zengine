#include <include/operators/model.h>
#include <include/base/helpers.h>

Model::Model()
	: Node(string("Model"))
	, TheShader(NodeType::SHADER, this, make_shared<string>("Shader"))
	, TheMesh(this, make_shared<string>("Mesh"))
	, Mapper(NULL)
{
	Type = NodeType::MODEL;
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

	static_cast<ShaderOperator*>(TheShader.GetConnectedNode())->Set();
	Mesh* mesh = TheMesh.Value();
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
		Mesh* mesh = TheMesh.Value();
		ShaderOperator* shader = (ShaderOperator*)TheShader.GetConnectedNode();
		if (mesh && shader)
		{
			Mapper = TheDrawingAPI->CreateAttributeMapper(mesh->Format->Attributes, 
				shader->ShaderProgram->Attributes, mesh->Format->Stride);
		}
	}
}

Model* Model::Create( ShaderOperator* ShaderOp, MeshOperator* MeshOp )
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
