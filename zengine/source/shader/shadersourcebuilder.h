#pragma once

#include <include/shader/shadersource2.h>

class ShaderSourceBuilder
{
public:
	static void FromStub(ShaderStub* Stub, ShaderSource2* Source);

private:
	ShaderSourceBuilder(ShaderStub* Stub, ShaderSource2* Source);
	~ShaderSourceBuilder();

	struct Depencency {
		Node* TheNode;

	};

	void						CollectMetadata();
	void						GenerateSource();

	static const string&		GetTypeString(NodeType Type);

	ShaderStub*					Stub;
	ShaderSource2*				Source;
};