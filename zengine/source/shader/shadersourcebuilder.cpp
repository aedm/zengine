#include "shadersourcebuilder.h"
#include <exception>

void ShaderSourceBuilder::FromStub(ShaderStub* Stub, ShaderSource2* Source)
{
	ShaderSourceBuilder builder(Stub, Source);
}

ShaderSourceBuilder::ShaderSourceBuilder(ShaderStub* _Stub, ShaderSource2* _Source)
	: Source(_Source)
{
	INFO("Building shader source...");
	SafeDelete(Source->metadata);
	for (Slot* slot : Source->mSlots) {
		if (slot != &Source->mStub) delete slot;
	}
	Source->mSlots.clear();
	Source->mSlots.push_back(&Source->mStub);

	if (_Stub == nullptr) return;

	try {
		CollectDependencies(_Stub);
		GenerateNames();
		for (Node* node : Dependencies)
		{
			if (node->GetType() == NodeType::SHADER_STUB)
			{
				CollectStubMetadata(node);
			}
			else {
				///NOT_IMPLEMENTED;
				/// TODO: generate uniforms here
			}
		}
		GenerateSlots();
		GenerateSource();
		_Source->metadata = new ShaderSourceMetadata(Inputs, Outputs, Uniforms);
	} catch (...) {
	}
}

ShaderSourceBuilder::~ShaderSourceBuilder()
{

}

void ShaderSourceBuilder::CollectStubMetadata(Node* Nd)
{
	ShaderStub* stub = static_cast<ShaderStub*>(Nd);

	ShaderStubMetadata* stubMeta = stub->GetStubMetadata();
	NodeData* data = DataMap.at(Nd);
	data->ReturnType = stubMeta->returnType;
	
	const map<ShaderStubParameter*, Slot*>& paramSlotMap = stub->GetParameterSlotMap();

	/// Globals
	for (auto global : stubMeta->globals)
	{
		Uniforms.push_back(new ShaderSourceUniform(
			global->type, global->name, nullptr, global->usage));
	}

	/// Inputs
	for (auto input : stubMeta->inputs)
	{
		/// TODO: check whether input types match
		if (InputsMap.find(input->name) == InputsMap.end())
		{
			InputsMap[input->name] = 
				new ShaderSourceVariable(input->type, input->name);
		}
	}

	/// Outputs
	for (auto output : stubMeta->outputs)
	{
		Outputs.push_back(new ShaderSourceVariable(output->type, output->name));
	}
}


void ShaderSourceBuilder::CollectDependencies(Node* Root)
{
	NodeData* data = new NodeData();
	DataMap[Root] = data;

	if (Root->GetType() == NodeType::SHADER_STUB)
	{
		for (Slot* slot : Root->mSlots)
		{
			Node* node = slot->GetNode();
			if (node == nullptr) {
				WARN("Incomplete shader graph.");
				throw exception();
			}
			if (DataMap.find(node) == DataMap.end())
			{
				CollectDependencies(node);
			}
		}
	}

	Dependencies.push_back(Root);
}

void ShaderSourceBuilder::GenerateNames()
{
	int uniformIndex = 0;
	int stubIndex = 0;
	char tmp[255]; // Fuk c++

	for (Node* node : Dependencies)
	{
		NodeData* data = DataMap.at(node);
		if (node->GetType() == NodeType::SHADER_STUB)
		{
			/// It's a function
			sprintf_s(tmp, "_func_%d", ++stubIndex);
			data->FunctionName = string(tmp);

			sprintf_s(tmp, "_var_%d", stubIndex);
			data->VariableName = string(tmp);
		} else {
			/// It's a uniform
			sprintf_s(tmp, "_uniform_%d", ++uniformIndex);
			data->VariableName = string(tmp);
		}
	}
}

void ShaderSourceBuilder::GenerateSourceMetadata()
{
	vector<ShaderSourceVariable*> inputs;
	for (auto input : InputsMap)
	{
		inputs.push_back(input.second);
	}
	Source->metadata = new ShaderSourceMetadata(inputs, Outputs, Uniforms);
}

void ShaderSourceBuilder::GenerateSlots()
{
	for (Node* node : Dependencies) 
	{
		if (node->GetType() != NodeType::SHADER_STUB) 
		{
			NodeData* data = DataMap.at(node);
			Slot* slot = new Slot(node->GetType(), Source, nullptr, false, true);
			slot->Connect(node);
			Uniforms.push_back(new ShaderSourceUniform(
				node->GetType(), data->VariableName, node, ShaderGlobalType::LOCAL));
		}
	}

	Source->ReceiveMessage(nullptr, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);
}

void ShaderSourceBuilder::GenerateSource()
{
	Source->mSource.clear();
	stringstream stream;
	stream << "#version 150" << endl;

	GenerateSourceHeader(stream);
	GenerateSourceFunctions(stream);
	GenerateSourceMain(stream);

	Source->mSource = stream.str();
}

void ShaderSourceBuilder::GenerateSourceHeader(stringstream& stream)
{
	/// Inputs
	for (auto var : InputsMap)
	{
		stream << "in " << GetTypeString(var.second->type) << ' ' << 
			var.second->name << ';' << endl;
		Inputs.push_back(var.second);
	}

	/// Outputs
	for (auto var : Outputs)
	{
		stream << "out " << GetTypeString(var->type) << ' ' << 
			var->name << ';' << endl;
	}

	/// Uniforms
	for (auto uniform : Uniforms)
	{
		stream << "uniform " << GetTypeString(uniform->type) << ' ' << 
			uniform->name << ';' << endl;
	}
}

void ShaderSourceBuilder::GenerateSourceFunctions(stringstream& stream)
{
	for (Node* node : Dependencies) 
	{
		if (node->GetType() == NodeType::SHADER_STUB)
		{
			NodeData* data = DataMap.at(node);
			ShaderStub* stub = static_cast<ShaderStub*>(node);
			ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

			stream << endl;
			stream << "#define SHADER " << GetTypeString(stubMeta->returnType) << 
				' ' << data->FunctionName << "(";
			for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
				ShaderStubParameter* param = stubMeta->parameters[i];
				stream << GetTypeString(param->type) << ' ' << param->name;
				if (i < stubMeta->parameters.size() - 1) stream << ", ";
			}
			stream << ')' << endl;
			stream << stubMeta->strippedSource;
			stream << "#undef SHADER" << endl;
		}
	}
}


void ShaderSourceBuilder::GenerateSourceMain(stringstream& stream)
{
	stream << endl;
	stream << "void main() {" << endl;
	for (Node* node : Dependencies)
	{
		if (node->GetType() == NodeType::SHADER_STUB)
		{
			NodeData* data = DataMap.at(node);
			ShaderStub* stub = static_cast<ShaderStub*>(node);
			ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

			stream << "  ";
			if (data->ReturnType != NodeType::NONE)
			{
				stream << GetTypeString(data->ReturnType) << ' ' <<
					data->VariableName << " = ";
			}
			stream << data->FunctionName << "(";
			for (UINT i = 0; i < stubMeta->parameters.size(); i++) 
			{
				ShaderStubParameter* param = stubMeta->parameters[i];
				Slot* slot = stub->GetParameterSlotMap().at(param);
				Node* paramNode = slot->GetNode();
				if (paramNode == nullptr) {
					/// Node not connected to param
					ERR("Parameter not connected");
					throw exception();
				}
				NodeData* paramData = DataMap.at(paramNode);
				stream << paramData->VariableName;
				if (i < stubMeta->parameters.size() - 1) stream << ", ";
			}
			stream << ");" << endl;
		}
	}
	stream << "}" << endl;
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
