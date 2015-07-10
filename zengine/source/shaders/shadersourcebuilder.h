#pragma once

#include <include/shaders/shadernode.h>
#include <sstream>

class ShaderSourceBuilder
{
public:
	static void FromStub(StubNode* stub, ShaderNode* shader);

private:
  ShaderSourceBuilder(StubNode* stub, ShaderNode* shader);
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
	void CollectDependencies(Node* root);
	
	/// Generates function and variable names
	void GenerateNames();

	/// Generate metadata
	void CollectStubMetadata(Node* node);
  void GenerateSourceMetadata();
	void GenerateSlots();

	/// Generate source
	void GenerateSource();
	void GenerateSourceHeader(stringstream& stream);
	void GenerateSourceFunctions(stringstream& stream);
	void GenerateSourceMain(stringstream& stream);

	static const string& GetTypeString(NodeType type);

	/// Topologic order of dependencies
	vector<Node*> mDependencies;

	/// Metadata for analyzing nodes
	map<Node*, NodeData*> mDataMap;

  ShaderNode* mShader;

	/// Metadata
	map<string, ShaderSourceVariable*> mInputsMap;
	vector<ShaderSourceVariable*>	mInputs;
	vector<ShaderSourceVariable*>	mOutputs;
  vector<ShaderSourceUniform*> mUniforms;
  vector<ShaderSourceUniform*> mSamplers;
};