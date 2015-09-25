#include "stubanalyzer.h"

OWNERSHIP StubMetadata* StubAnalyzer::FromText(const char* stubSource) {
  StubAnalyzer analyzer(stubSource);

  if (analyzer.mName == nullptr) {
    ERR("Shader stub has no name.");
    WARN("source:\n%s", stubSource);
    return nullptr;
  }

  return new StubMetadata(*analyzer.mName, analyzer.mReturnType,
                                analyzer.mStrippedSource, analyzer.mParameters, 
                                analyzer.mGlobals,
                                analyzer.mInputs, analyzer.mOutputs);
}

StubAnalyzer::StubAnalyzer(const char* stubSource)
  : mName(nullptr)
  , mCurrentLineNumber(-1)
  , mReturnType(NodeType::NONE) {
  vector<SourceLine*>* lines = SplitToWords(stubSource);
  for (SourceLine* line : *lines) {
    mCurrentLineNumber = line->LineNumber;
    if (line->SubStrings[0].Token == TOKEN_COLON) {
      AnalyzeMetadataLine(line);
    } else {
      mStrippedSource = mStrippedSource + line->EntireLine.ToString() + '\n';
    }
    delete line;
  }
  delete lines;
}

void StubAnalyzer::AnalyzeMetadataLine(SourceLine* line) {
  if (line->SubStrings.size() < 2) {
    ERR("line %d: Line empty.", mCurrentLineNumber);
    return;
  }

  switch (line->SubStrings[1].Token) {
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
      AnalyzeVariable(line, mInputs);
      break;
    case TOKEN_output:
      AnalyzeVariable(line, mOutputs);
      break;
    default:
      ERR("line %d: unknown metadata type '%s'.", mCurrentLineNumber,
          line->SubStrings[1].ToString().c_str());
      break;
  }
}

void StubAnalyzer::AnalyzeName(SourceLine* line) {
  if (line->SubStrings.size() != 3 || line->SubStrings[2].Token != TOKEN_STRING) {
    ERR("line %d: Wrong syntax, use ':name \"<name>\"'", mCurrentLineNumber);
    return;
  }

  mName = line->SubStrings[2].ToStringPtr();
}

void StubAnalyzer::AnalyzeReturns(SourceLine* line) {
  if (line->SubStrings.size() != 3) {
    ERR("line %d: Wrong syntax, use ':returns [void|float|vec2|vec3|vec4]'",
        mCurrentLineNumber);
    return;
  }

  mReturnType = TokenToType(line->SubStrings[2]);
}

void StubAnalyzer::AnalyzeParam(SourceLine* line) {
  if (line->SubStrings.size() < 4) {
    ERR("line %d: Wrong syntax, use ':param <type> <name>'",
        mCurrentLineNumber);
    return;
  }

  StubParameter* parameter = new StubParameter();
  parameter->mType = TokenToType(line->SubStrings[2]);
  parameter->mName = SharedString(line->SubStrings[3].ToStringPtr());
  mParameters.push_back(parameter);
}


void StubAnalyzer::AnalyzeVariable(SourceLine* line, vector<StubVariable*>& Storage) {
  if (line->SubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  StubVariable* parameter = new StubVariable();
  parameter->type = TokenToType(line->SubStrings[2]);
  parameter->name = line->SubStrings[3].ToString();
  Storage.push_back(parameter);
}


void StubAnalyzer::AnalyzeGlobal(SourceLine* line) {
  if (line->SubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  SubString& name = line->SubStrings[3];
  int usage =
    EnumMapperA::GetEnumFromString(GlobalUniformMapper, name.Begin, name.Length);
  if (usage < 0) {
    ERR("line %d: Unrecognized global uniform '%s'.", mCurrentLineNumber,
        name.ToString().c_str());
    return;
  }

  NodeType declaredType = TokenToType(line->SubStrings[2]);
  NodeType expectedType = GlobalUniformTypes[usage];
  if (declaredType != expectedType) {
    ERR("line %d: wrong type for global uniform '%s'.", mCurrentLineNumber,
        name.ToString().c_str());
    return;
  }

  StubGlobal* global = new StubGlobal();
  global->name = name.ToString();
  global->type = declaredType;
  global->usage = (ShaderGlobalType)usage;
  mGlobals.push_back(global);
}


NodeType StubAnalyzer::TokenToType(const SubString& subStr) {
  switch (subStr.Token) {
    case TOKEN_void:		return NodeType::NONE;
    case TOKEN_float:		return NodeType::FLOAT;
    case TOKEN_vec2:		return NodeType::VEC2;
    case TOKEN_vec3:		return NodeType::VEC3;
    case TOKEN_vec4:		return NodeType::VEC4;
    case TOKEN_mat4:		return NodeType::MATRIX44;
    case TOKEN_sampler2D:	return NodeType::TEXTURE;
    default:
      ERR("line %d: Wrong type '%s'",
          mCurrentLineNumber, subStr.ToString().c_str());
      return NodeType::NONE;
  }
}
