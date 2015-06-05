#pragma once

#include <include/shader/shadersource2.h>

class ShaderSourceBuilder
{
public:
	static void FromStub(ShaderStub* Stub, ShaderSource2* Source);

private:
	ShaderSourceBuilder(ShaderStub* Stub, ShaderSource2* Source);
	~ShaderSourceBuilder();

	struct NodeData
	{
	};

	void						CollectDependencies(Node* Root);
	void						CollectMetadata();
	void						GenerateSource();

	static const string&		GetTypeString(NodeType Type);

	/// Topologic order of dependencies
	vector<Node*>				Dependencies;

	/// Metadata for analyzing nodes
	map<Node*, NodeData*>		DataMap;

	ShaderStub*					Stub;
	ShaderSource2*				Source;
};