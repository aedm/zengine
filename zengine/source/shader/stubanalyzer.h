#pragma once

#include <include/shader/shaderstub.h>
#include "../shaders/shaderTokenizer.h"

/// TODO: Remove this.
using namespace Shaders;

class StubAnalyzer
{
public:
	/// Run analysis for a shader in stub source format.
	/// Returns user-defined metadata extraced from source.
	static OWNERSHIP ShaderStubMetadata*	FromText(const char* StubSource);
	
private:
	StubAnalyzer(const char* StubSource);

	void							AnalyzeMetadataLine(SourceLine* line);
	
	void							AnalyzeName(SourceLine* line);
	void							AnalyzeReturns(SourceLine* line);
	void							AnalyzeParam(SourceLine* line);
	void							AnalyzeGlobal(SourceLine* line);
	void							AnalyzeVariable(SourceLine* line, 
										vector<ShaderStubVariable*>& Storage);

	NodeType						TokenToType(const SubString& SubStr);

	int								CurrentLineNumber;
	
	/// Builders
	string*							Name;
	string							StrippedSource;
	NodeType						ReturnType;
	vector<ShaderStubParameter*>	Parameters;
	vector<ShaderStubGlobal*>		Globals;
	vector<ShaderStubVariable*>		Inputs;
	vector<ShaderStubVariable*>		Outputs;
};