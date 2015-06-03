#include "stubanalyzer.h"

OWNERSHIP ShaderStubMetadata* StubAnalyzer::FromText(const char* StubSource)
{
	StubAnalyzer analyzer(StubSource);

	if (analyzer.Name == nullptr) {
		ERR("Shader stub has no name.");
		return nullptr;
	}

	return new ShaderStubMetadata(*analyzer.Name, analyzer.ReturnType,
		analyzer.StrippedSource, analyzer.Parameters, analyzer.Globals, 
		analyzer.Inputs, analyzer.Outputs);
}

StubAnalyzer::StubAnalyzer(const char* StubSource)
	: Name(nullptr)
	, CurrentLineNumber(-1)
	, ReturnType(NodeType::NONE)
{
	vector<SourceLine*>* lines = SplitToWords(StubSource);
	for (SourceLine* line : *lines)
	{
		CurrentLineNumber = line->LineNumber;
		if (line->SubStrings[0].Token == TOKEN_COLON)
		{
			AnalyzeMetadataLine(line);
		} else {
			StrippedSource = StrippedSource + line->EntireLine.ToString() + '\n';
		}
		delete line;
	}
	delete lines;
}

void StubAnalyzer::AnalyzeMetadataLine(SourceLine* line)
{
	if (line->SubStrings.size() < 2) {
		ERR("line %d: Line empty.", CurrentLineNumber);
		return;
	}

	switch (line->SubStrings[1].Token)
	{
	case TOKEN_name:
		AnalyzeName(line);
		break;
	case TOKEN_returns:
		AnalyzeReturns(line);
		break;
	case TOKEN_param:
		AnalyzeParam(line);
		break;
	case TOKEN_global:
		AnalyzeGlobal(line);
		break;
	case TOKEN_input:
		AnalyzeVariable(line, Inputs);
		break;
	case TOKEN_output:
		AnalyzeVariable(line, Outputs);
		break;
	default:
		ERR("line %d: unknown metadata type '%s'.", CurrentLineNumber,
			line->SubStrings[1].ToString().c_str());
		break;
	}
}

void StubAnalyzer::AnalyzeName(SourceLine* line)
{
	if (line->SubStrings.size() != 3 || line->SubStrings[2].Token != TOKEN_STRING) {
		ERR("line %d: Wrong syntax, use ':name \"<name>\"'", CurrentLineNumber);
		return;
	}

	Name = line->SubStrings[2].ToStringPtr();
}

void StubAnalyzer::AnalyzeReturns(SourceLine* line)
{
	if (line->SubStrings.size() != 3) {
		ERR("line %d: Wrong syntax, use ':returns [void|float|vec2|vec3|vec4]'", 
			CurrentLineNumber);
		return;
	}

	ReturnType = TokenToType(line->SubStrings[2]);
}

void StubAnalyzer::AnalyzeParam(SourceLine* line)
{
	if (line->SubStrings.size() < 4) {
		ERR("line %d: Wrong syntax, use ':param <type> <name>'",
			CurrentLineNumber);
		return;
	}

	ShaderStubParameter* parameter = new ShaderStubParameter();
	parameter->Type = TokenToType(line->SubStrings[2]);
	parameter->Name = line->SubStrings[3].ToString();
	Parameters.push_back(parameter);
}


void StubAnalyzer::AnalyzeVariable(SourceLine* line, vector<ShaderStubVariable*>& Storage)
{
	if (line->SubStrings.size() < 4) {
		ERR("line %d: Wrong syntax", CurrentLineNumber);
		return;
	}

	ShaderStubVariable* parameter = new ShaderStubVariable();
	parameter->Type = TokenToType(line->SubStrings[2]);
	parameter->Name = line->SubStrings[3].ToString();
	Storage.push_back(parameter);
}


void StubAnalyzer::AnalyzeGlobal(SourceLine* line)
{
	if (line->SubStrings.size() < 4) {
		ERR("line %d: Wrong syntax", CurrentLineNumber);
		return;
	}

	SubString& name = line->SubStrings[3];
	int usage = 
		EnumMapperA::GetEnumFromString(GlobalUniformMapper, name.Begin, name.Length);
	if (usage < 0)
	{
		ERR("line %d: Unrecognized global uniform '%s'.", CurrentLineNumber, 
			name.ToString().c_str());
		return;
	}
	
	NodeType declaredType = TokenToType(line->SubStrings[2]);
	NodeType expectedType = GlobalUniformTypes[usage];
	if (declaredType != expectedType) 
	{
		ERR("line %d: wrong type for global uniform '%s'.", CurrentLineNumber,
			name.ToString().c_str());
		return;
	}

	ShaderStubGlobal* global = new ShaderStubGlobal();
	global->Name = name.ToString();
	global->Type = declaredType;
	global->Usage = (ShaderGlobalType)usage;
	Globals.push_back(global);
}


NodeType StubAnalyzer::TokenToType(const SubString& SubStr)
{
	switch (SubStr.Token)
	{
	case TOKEN_void:		return NodeType::NONE;
	case TOKEN_float:		return NodeType::FLOAT;
	case TOKEN_vec2:		return NodeType::VEC2;
	case TOKEN_vec3:		return NodeType::VEC3;
	case TOKEN_vec4:		return NodeType::VEC4;
	case TOKEN_mat4:		return NodeType::MATRIX44;
	case TOKEN_sampler2d:	return NodeType::TEXTURE;
	default:
		ERR("line %d: Wrong type '%s'",
			CurrentLineNumber, SubStr.ToString().c_str());
		return NodeType::NONE;
	}
}
