#pragma once

#include <include/shader/shadersource2.h>
#include <sstream>

class ShaderSourceBuilder
{
public:
	static void FromStub(ShaderStub* Stub, ShaderSource2* Source);

private:
	ShaderSourceBuilder(ShaderStub* Stub, ShaderSource2* Source);
	~ShaderSourceBuilder();

	struct NodeData
	{
		/// The variable name in the main function,
		/// or the uniform name if the node isn't a stub.
		string VariableName;

		/// Function name for stubs
		string FunctionName;

		/// Stub return type
		NodeType ReturnType;
	};

	/// Creates topological order of dependency tree
	void						CollectDependencies(Node* Root);
	
	/// Generates function and variable names
	void						GenerateNames();

	/// Generate metadata
	void						CollectStubMetadata(Node* Nd);
	void						GenerateSourceMetadata();
	void						GenerateSlots();

	/// Generate source
	void						GenerateSource();
	void						GenerateSourceHeader(stringstream& stream);
	void						GenerateSourceFunctions(stringstream& stream);
	void						GenerateSourceMain(stringstream& stream);

	static const string&		GetTypeString(NodeType Type);

	/// Topologic order of dependencies
	vector<Node*>				Dependencies;

	/// Metadata for analyzing nodes
	map<Node*, NodeData*>		DataMap;

	//ShaderStub*					Stub;
	ShaderSource2*				Source;

	/// Metadata
	map<string, ShaderSourceVariable*>	InputsMap;
	vector<ShaderSourceVariable*>	Inputs;
	vector<ShaderSourceVariable*>	Outputs;
	vector<ShaderSourceUniform*>	Uniforms;
};