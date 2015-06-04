#include <include/shader/drawable.h>


Drawable::Drawable()
	: Node(NodeType::PASS, "Drawable")
	, TheMesh(this, make_shared<string>("Mesh"))
	, TheMaterial(NodeType::MATERIAL, this, make_shared<string>("Material"))
{}

Drawable::~Drawable()
{}

void Drawable::Draw(Globals* Global)
{
	if (!IsProperlyConnected) return;
	Material* material = static_cast<Material*>(TheMaterial.GetNode());
	const Mesh* mesh = TheMesh.GetMesh();

	/// Set pass (pipeline state)
	Pass* pass = material->GetPass();
	pass->Set(Global);

	/// Set vertex buffer and attributes
	TheDrawingAPI->SetVertexBuffer(mesh->VertexHandle);
	for (ShaderAttributeDesc desc : pass->GetUsedAttributes())
	{
		VertexAttribute* attribute = mesh->Format->AttributesArray[(UINT)desc.Usage];
		if (attribute != nullptr) 
		{
			TheDrawingAPI->EnableVertexAttribute(desc.Handle, 
				VertexAttributeType[(UINT)desc.Usage], attribute->Offset, 
				mesh->Format->Stride);
		}
	}

	/// TODO: set output buffers

	/// Render mesh
	if (mesh->IndexHandle)
	{
		TheDrawingAPI->Render(mesh->IndexHandle, mesh->IndexCount, PRIMITIVE_TRIANGLES);
	} else {
		TheDrawingAPI->Render(0, mesh->VertexCount, PRIMITIVE_TRIANGLES);
	}
}
