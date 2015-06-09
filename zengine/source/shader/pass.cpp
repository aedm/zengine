#include <include/shader/pass.h>
#include <include/shader/shaderstub.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

Pass::Pass()
	: Node(NodeType::PASS, "Pass")
	, VertexStub(NodeType::SHADER_STUB, this, make_shared<string>("Vertex shader"))
	, FragmentStub(NodeType::SHADER_STUB, this, make_shared<string>("Fragment shader"))
	, VertexSource(NodeType::SHADER_SOURCE, this, nullptr, false, false)
	, FragmentSource(NodeType::SHADER_SOURCE, this, nullptr, false, false)
	, Handle(-1)
{
}

Pass::~Pass()
{}

void Pass::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		if (S == &VertexStub) {
			if (VertexStub.GetNode() == nullptr) {
				VertexSource.Connect(nullptr);
			} else {
				VertexSource.Connect(static_cast<ShaderStub*>(
					VertexStub.GetNode())->GetShaderSource());
			}
		}
		else if (S == &FragmentStub) {
			if (FragmentStub.GetNode() == nullptr) {
				FragmentSource.Connect(nullptr);
			} else {
				FragmentSource.Connect(static_cast<ShaderStub*>(
					FragmentStub.GetNode())->GetShaderSource());
			}
		}
		break;
	case NodeMessage::VALUE_CHANGED:
		if (S == &VertexSource || S == &FragmentSource) {
			BuildRenderPipeline();
		}
		break;
	default: break;
	}
}

void Pass::BuildRenderPipeline()
{
	INFO("Building render pipeline...");
	Uniforms.clear();
	Attributes.clear();
	Handle = -1;

	//ShaderSource2* vertex = static_cast<ShaderSource2*>(VertexShader.GetNode());
	//ShaderSource2* fragment = static_cast<ShaderSource2*>(FragmentShader.GetNode());
	ShaderStub* vertexStub = static_cast<ShaderStub*>(VertexStub.GetNode());
	ShaderStub* fragmentStub = static_cast<ShaderStub*>(FragmentStub.GetNode());
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
	ASSERT(Handle != -1);

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

bool Pass::IsComplete()
{
	return Handle != -1;
}

