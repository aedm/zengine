#include "shadersourcebuilder.h"
#include <sstream>

void ShaderSourceBuilder::FromStub(ShaderStub* Stub, ShaderSource2* Source)
{
	ShaderSourceBuilder builder(Stub, Source);
}

ShaderSourceBuilder::ShaderSourceBuilder(ShaderStub* _Stub, ShaderSource2* _Source)
	: Stub(_Stub)
	, Source(_Source)
{
	CollectMetadata();
}

ShaderSourceBuilder::~ShaderSourceBuilder()
{

}

void ShaderSourceBuilder::CollectMetadata()
{
	SafeDelete(Source->Metadata);
	//ShaderStub* stub = static_cast<ShaderStub*>(Stub.GetNode());
	if (Stub == nullptr) return;

	ShaderStubMetadata* stubMeta = Stub->GetStubMetadata();
	const map<ShaderStubParameter*, Slot*>& paramSlotMap = Stub->GetParameterSlotMap();

	for (Slot* slot : Source->Slots) {
		if (slot != &Source->Stub) delete slot;
	}
	Source->Slots.clear();
	Source->Slots.push_back(&Source->Stub);
	Source->ReceiveMessage(nullptr, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);

	/// Uniforms
	vector<ShaderSourceUniform*> uniforms;
	for (auto param : stubMeta->Parameters)
	{
		Slot* stubSlot = paramSlotMap.at(param);
		Slot* slot = 
			new Slot(stubSlot->GetType(), Source, stubSlot->GetName(), false, true);
		slot->Connect(stubSlot->GetNode());

		uniforms.push_back(new ShaderSourceUniform(
			param->Type, param->Name, slot, ShaderGlobalType::LOCAL));
	}

	/// Globals
	for (auto global : stubMeta->Globals)
	{
		uniforms.push_back(new ShaderSourceUniform(
			global->Type, global->Name, nullptr, global->Usage));
	}

	/// Inputs
	vector<ShaderSourceVariable*> inputs;
	for (auto output : stubMeta->Inputs)
	{
		inputs.push_back(new ShaderSourceVariable(output->Type, output->Name));
	}

	/// Outputs
	vector<ShaderSourceVariable*> outputs;
	for (auto output : stubMeta->Outputs)
	{
		outputs.push_back(new ShaderSourceVariable(output->Type, output->Name));
	}

	Source->Metadata = new ShaderSourceMetadata(inputs, outputs, uniforms);

	GenerateSource();
}

void ShaderSourceBuilder::GenerateSource()
{
	Source->Source.clear();

	stringstream stream;

	stream << "#version 150" << endl;

	/// Inputs
	for (auto var : Source->Metadata->Inputs)
	{
		stream << "in " << GetTypeString(var->Type) << ' ' << var->Name << ';' << endl;
	}

	/// Outputs
	for (auto var : Source->Metadata->Outputs)
	{
		stream << "out " << GetTypeString(var->Type) << ' ' << var->Name << ';' << endl;
	}

	/// Uniforms
	for (auto uniform : Source->Metadata->Uniforms)
	{
		stream << "uniform " << GetTypeString(uniform->Type) << ' ' << uniform->Name << ';' << endl;
	}

	/// Code
	//ShaderStub* stub = static_cast<ShaderStub*>(Source->Stub.GetNode());
	if (Stub == nullptr) return;
	ShaderStubMetadata* stubMeta = Stub->GetStubMetadata();

	stream << endl;
	stream << "#define SHADER " << GetTypeString(stubMeta->ReturnType) << " __func0(";
	for (UINT i = 0; i < stubMeta->Parameters.size(); i++) {
		ShaderStubParameter* param = stubMeta->Parameters[i];
		stream << GetTypeString(param->Type) << ' ' << param->Name;
		if (i < stubMeta->Parameters.size() - 1) stream << ", ";
	}
	stream << ')' << endl;
	stream << stubMeta->StrippedSource;
	stream << "#undef SHADER" << endl;

	/// Main function
	stream << endl;
	stream << "void main() {" << endl;
	stream << "  __func0();" << endl;
	stream << "}" << endl;

	Source->Source = stream.str();
}


const string& ShaderSourceBuilder::GetTypeString(NodeType Type)
{
	static const string sfloat("float");
	static const string svec2("vec2");
	static const string svec3("vec3");
	static const string svec4("vec4");
	static const string suint("uint");
	static const string smatrix44("mat4");
	static const string ssampler2d("sampler2d");
	static const string svoid("void");
	static const string serror("UNKNOWN_TYPE");

	switch (Type)
	{
	case NodeType::FLOAT:		return sfloat;
	case NodeType::VEC2:		return svec2;
	case NodeType::VEC3:		return svec3;
	case NodeType::VEC4:		return svec4;
	case NodeType::UINT:		return suint;
	case NodeType::MATRIX44:	return smatrix44;
	case NodeType::TEXTURE:		return ssampler2d;
	case NodeType::NONE:		return svoid;
	default:
		ERR("Unhandled type: %d", Type);
		return serror;
	}
}

void ShaderSourceBuilder::CollectDependencies(Node* Root)
{
	NodeData* data = new NodeData();
	DataMap[Root] = data;

	for (Slot* slot : Root->Slots) 
	{
		NOT_IMPLEMENTED;
	}
}
