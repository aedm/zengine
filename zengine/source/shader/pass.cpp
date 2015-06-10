#include <include/shader/pass.h>
#include <include/shader/shaderstub.h>
#include <include/render/drawingapi.h>
#include <include/nodes/valuenodes.h>

Pass::Pass()
	: Node(NodeType::PASS, "Pass")
	, mVertexStub(NodeType::SHADER_STUB, this, make_shared<string>("Vertex shader"))
	, mFragmentStub(NodeType::SHADER_STUB, this, make_shared<string>("Fragment shader"))
	, mVertexSource(NodeType::SHADER_SOURCE, this, nullptr, false, false)
	, mFragmentSource(NodeType::SHADER_SOURCE, this, nullptr, false, false)
	, mHandle(-1)
{
}

Pass::~Pass()
{}

void Pass::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		if (S == &mVertexStub) {
			if (mVertexStub.GetNode() == nullptr) {
				mVertexSource.Connect(nullptr);
			} else {
				mVertexSource.Connect(static_cast<ShaderStub*>(
					mVertexStub.GetNode())->GetShaderSource());
			}
		}
		else if (S == &mFragmentStub) {
			if (mFragmentStub.GetNode() == nullptr) {
				mFragmentSource.Connect(nullptr);
			} else {
				mFragmentSource.Connect(static_cast<ShaderStub*>(
					mFragmentStub.GetNode())->GetShaderSource());
			}
		}
		break;
	case NodeMessage::VALUE_CHANGED:
		if (S == &mVertexSource || S == &mFragmentSource) {
			BuildRenderPipeline();
		}
		break;
	default: break;
	}
}

void Pass::BuildRenderPipeline()
{
	INFO("Building render pipeline...");
	mUniforms.clear();
	mAttributes.clear();
	mHandle = -1;

	//ShaderSource2* vertex = static_cast<ShaderSource2*>(VertexShader.GetNode());
	//ShaderSource2* fragment = static_cast<ShaderSource2*>(FragmentShader.GetNode());
	//ShaderStub* vStub = static_cast<ShaderStub*>(vertexStub.GetNode());
	//ShaderStub* fStub = static_cast<ShaderStub*>(fragmentStub.GetNode());
	//if (vStub == nullptr || fStub == nullptr) return;

	//ShaderSource2* vertex = vStub->GetShaderSource();
	//ShaderSource2* fragment = fStub->GetShaderSource();
	//if (vertex == nullptr || vertex->GetMetadata() == nullptr ||
	//	fragment == nullptr || fragment->GetMetadata() == nullptr) return;

  ShaderSource2* vertex = static_cast<ShaderSource2*>(mVertexSource.GetNode());
  ShaderSource2* fragment = static_cast<ShaderSource2*>(mFragmentSource.GetNode());
  if (vertex == nullptr || vertex->GetMetadata() == nullptr ||
  	  fragment == nullptr || fragment->GetMetadata() == nullptr) return;

	const string& vertexSource = vertex->GetSource();
	const string& fragmentSource = fragment->GetSource();

	ShaderCompileDesc* shaderCompileDesc = TheDrawingAPI->CreateShaderFromSource(
		vertexSource.c_str(), fragmentSource.c_str());
	if (shaderCompileDesc == nullptr) return;

	mHandle = shaderCompileDesc->Handle;

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
		passUniform.handle = uniformDesc.Handle;
		passUniform.node = sourceUniform->TheNode;
		passUniform.globalType = sourceUniform->GlobalType;
		passUniform.type = sourceUniform->Type;
		mUniforms.push_back(passUniform);
	}
	
	/// Collect required attributes
	for (auto attributeDesc : shaderCompileDesc->Attributes) {
		mAttributes.push_back(attributeDesc);
	}

}

void Pass::Set(Globals* Global)
{
	ASSERT(mHandle != -1);

	TheDrawingAPI->SetShaderProgram(mHandle);

	/// Set uniforms
	for (PassUniform& uniform : mUniforms)
	{
		if (uniform.globalType == ShaderGlobalType::LOCAL) {
			/// Local uniform, takes value from a slot
			ASSERT(uniform.node != nullptr);
			switch (uniform.type)
			{
			#undef ITEM
			#define ITEM(name, type, token) \
				case NodeType::name: \
					TheDrawingAPI->SetUniform(uniform.handle, NodeType::name, \
						&static_cast<ValueNode<NodeType::name>*>(uniform.node)->Get()); \
					break;
			VALUETYPE_LIST

			default: SHOULDNT_HAPPEN; break;
			}
		} else {
			/// Global uniform, takes value from the Globals object
			int offset = GlobalUniformOffsets[(UINT)uniform.globalType];
			void* source = reinterpret_cast<char*>(Global)+offset;
			TheDrawingAPI->SetUniform(uniform.handle, uniform.type, source); 
		}
	}
}

const vector<ShaderAttributeDesc>& Pass::GetUsedAttributes()
{
	return mAttributes;
}

Node* Pass::Clone() const
{
	return new Pass();
}

bool Pass::isComplete()
{
	return mHandle != -1;
}

