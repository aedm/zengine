#include <include/shader/shadersource2.h>
#include <include/shader/shaderstub.h>
#include <sstream>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, Stub(NodeType::SHADER_STUB, this, make_shared<string>("Stub"))
	, Metadata(nullptr)
{}

ShaderSource2::~ShaderSource2()
{
	SafeDelete(Metadata);
}


Node* ShaderSource2::Clone() const
{
	/// Shader sources get all their data from the slot, so nothing to do here.
	return new ShaderSource2();
}


void ShaderSource2::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
	case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
		if (S == &Stub) CollectMetadata();
		break;
		//case NodeMessage::VALUE_CHANGED:
		//	break;
		//case NodeMessage::NEEDS_REDRAW:
		//	break;
	default:
		break;
	}
}

void ShaderSource2::CollectMetadata()
{
	SafeDelete(Metadata);
	ShaderStub* stub = static_cast<ShaderStub*>(Stub.GetNode());
	if (stub == nullptr) return;

	ShaderStubMetadata* stubMeta = stub->GetStubMetadata();
	const map<ShaderStubParameter*, Slot*>& paramSlotMap = stub->GetParameterSlotMap();
	
	for (Slot* slot : Slots) {
		if (slot != &Stub) delete slot;
	}
	Slots.clear();
	Slots.push_back(&Stub);
	ReceiveMessage(nullptr, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);

	/// Uniforms
	vector<ShaderSourceUniform*> uniforms;
	for (auto param : stubMeta->Parameters) 
	{
		Slot* stubSlot = paramSlotMap.at(param);
		Slot* slot = new Slot(stubSlot->GetType(), this, stubSlot->GetName(), false, true);
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

	Metadata = new ShaderSourceMetadata(inputs, outputs, uniforms);

	GenerateSource();
}

void ShaderSource2::GenerateSource()
{
	Source.clear();

	stringstream stream;

	stream << "#version 150" << endl;

	/// Inputs
	for (auto var : Metadata->Inputs)
	{
		stream << "in " << GetTypeString(var->Type) << ' ' << var->Name << ';' << endl;
	}
	
	/// Outputs
	for (auto var : Metadata->Outputs)
	{
		stream << "out " << GetTypeString(var->Type) << ' ' << var->Name << ';' << endl;
	}

	/// Uniforms
	for (auto uniform : Metadata->Uniforms)
	{
		stream << "uniform " << GetTypeString(uniform->Type) << ' ' << uniform->Name << ';' << endl;
	}

	/// Code
	ShaderStub* stub = static_cast<ShaderStub*>(Stub.GetNode());
	if (stub == nullptr) return;
	ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

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

	Source = stream.str();
}


const string& ShaderSource2::GetTypeString(NodeType Type)
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

const string& ShaderSource2::GetSource() const
{
	return Source;
}

const ShaderSourceMetadata* ShaderSource2::GetMetadata() const
{
	return Metadata;
}


ShaderSourceMetadata::ShaderSourceMetadata(
	const vector<OWNERSHIP ShaderSourceVariable*>& _Inputs,
	const vector<OWNERSHIP ShaderSourceVariable*>& _Outputs,
	const vector<OWNERSHIP ShaderSourceUniform*>& _Uniforms)
	: Inputs(_Inputs)
	, Outputs(_Outputs)
	, Uniforms(_Uniforms)
{}

ShaderSourceMetadata::~ShaderSourceMetadata()
{
	for (auto x : Inputs) delete x;
	for (auto x : Outputs) delete x;
	for (auto x : Uniforms) delete x;
}


ShaderSourceVariable::ShaderSourceVariable(NodeType _Type, const string& _Name)
	: Name(_Name)
	, Type(_Type)
{}

ShaderSourceUniform::ShaderSourceUniform(NodeType _Type, const string& _Name, Slot* _slot, 
	ShaderGlobalType _GlobalType)
	: Type(_Type)
	, Name(_Name)
	, TheSlot(_slot)
	, GlobalType(_GlobalType)
{}
