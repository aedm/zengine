#include <include/shader/pass.h>
#include <include/shader/shaderstub.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

Pass::Pass()
	: Node(NodeType::PASS, "Pass")
	, VertexShader(NodeType::SHADER_STUB, this, make_shared<string>("Vertex shader"))
	, FragmentShader(NodeType::SHADER_STUB, this, make_shared<string>("Fragment shader"))
{
}

Pass::~Pass()
{}

void Pass::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
	case NodeMessage::VALUE_CHANGED:
		BuildRenderPipeline();
		break;
	default: break;
	}
}

void Pass::BuildRenderPipeline()
{
	Uniforms.clear();
	Attributes.clear();
	Handle = 0;

	//ShaderSource2* vertex = static_cast<ShaderSource2*>(VertexShader.GetNode());
	//ShaderSource2* fragment = static_cast<ShaderSource2*>(FragmentShader.GetNode());
	ShaderStub* vertexStub = static_cast<ShaderStub*>(VertexShader.GetNode());
	ShaderStub* fragmentStub = static_cast<ShaderStub*>(FragmentShader.GetNode());
	if (vertexStub == nullptr || fragmentStub == nullptr) return;

	ShaderSource2* vertex = vertexStub->GetShaderSource();
	ShaderSource2* fragment = fragmentStub->GetShaderSource();
	if (vertex == nullptr || vertex->GetMetadata() == nullptr ||
		fragment == nullptr || fragment->GetMetadata() == nullptr) return;

	const string& vertexSource = vertex->GetSource();
	const string& fragmentSource = fragment->GetSource();

	ShaderCompileDesc* shaderCompileDesc = TheDrawingAPI->CreateShaderFromSource(
		vertexSource.c_str(), fragmentSource.c_str());
	if (shaderCompileDesc == nullptr) return;

	Handle = shaderCompileDesc->Handle;

	/// Collect uniforms from shader stage sources
	map<string, ShaderSourceUniform*> uniformMap;
	for (auto uniform : vertex->GetMetadata()->Uniforms)
	{
		uniformMap[uniform->Name] = uniform;
	}

	for (auto uniform : fragment->GetMetadata()->Uniforms)
	{
		uniformMap[uniform->Name] = uniform;
	}

	/// Merge uniform info
	for (auto uniformDesc : shaderCompileDesc->Uniforms) 
	{
		ShaderSourceUniform* sourceUniform = uniformMap.at(uniformDesc.Name);
		PassUniform passUniform;
		passUniform.Handle = uniformDesc.Handle;
		passUniform.TheNode = sourceUniform->TheNode;
		passUniform.GlobalType = sourceUniform->GlobalType;
		passUniform.Type = sourceUniform->Type;
		Uniforms.push_back(passUniform);
	}
	
	/// Collect required attributes
	for (auto attributeDesc : shaderCompileDesc->Attributes) {
		Attributes.push_back(attributeDesc);
	}

}

void Pass::Set(Globals* Global)
{
	TheDrawingAPI->SetShaderProgram(Handle);

	/// Set uniforms
	for (PassUniform& uniform : Uniforms)
	{
		if (uniform.GlobalType == ShaderGlobalType::LOCAL) {
			/// Local uniform, takes value from a slot
			ASSERT(uniform.TheNode != nullptr);
			switch (uniform.Type)
			{
			#undef ITEM
			#define ITEM(name, type, token) \
				case NodeType::name: \
					TheDrawingAPI->SetUniform(uniform.Handle, NodeType::name, \
						&static_cast<ValueNode<NodeType::name>*>(uniform.TheNode)->Get()); \
					break;
			VALUETYPE_LIST

			default: SHOULDNT_HAPPEN; break;
			}
		} else {
			/// Global uniform, takes value from the Globals object
			int offset = GlobalUniformOffsets[(UINT)uniform.GlobalType];
			void* source = reinterpret_cast<char*>(Global)+offset;
			TheDrawingAPI->SetUniform(uniform.Handle, uniform.Type, source); 
		}
	}
}

const vector<ShaderAttributeDesc>& Pass::GetUsedAttributes()
{
	return Attributes;
}

Node* Pass::Clone() const
{
	return new Pass();
}

