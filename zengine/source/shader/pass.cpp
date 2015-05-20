#include <include/shader/pass.h>
#include <include/render/drawingapi.h>

Pass::Pass()
	: Node(NodeType::PASS, "Pass")
	, VertexShader(NodeType::SHADER_SOURCE, this, make_shared<string>("Vertex shader"))
	, FragmentShader(NodeType::SHADER_SOURCE, this, make_shared<string>("Fragment shader"))
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
	UniformMap.clear();

	ShaderSource2* vertex = static_cast<ShaderSource2*>(VertexShader.GetNode());
	ShaderSource2* fragment = static_cast<ShaderSource2*>(FragmentShader.GetNode());

	if (vertex == nullptr || fragment == nullptr) return;

	const string& vertexSource = vertex->GetSource();
	const string& fragmentSource = fragment->GetSource();

	ShaderCompileDesc* shaderCompileDesc =
		TheDrawingAPI->CreateShaderFromSource(vertexSource.c_str(), fragmentSource.c_str());
	if (shaderCompileDesc == nullptr) return;

	for (auto uniform : vertex->GetMetadata()->Uniforms) 
	{
		UniformMap[uniform->Name] = uniform;
		Uniforms.push_back(uniform);
	}

	for (auto uniform : fragment->GetMetadata()->Uniforms)
	{
		UniformMap[uniform->Name] = uniform;
		Uniforms.push_back(uniform);
	}

	
}

