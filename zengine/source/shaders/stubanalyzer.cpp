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
                          analyzer.mGlobalUniforms, analyzer.mGlobalSamplers,
                          analyzer.mInputs, analyzer.mOutputs);
}

StubAnalyzer::StubAnalyzer(const char* stubSource)
  : mName(nullptr)
  , mCurrentLineNumber(-1)
  , mReturnType(StubParameter::Type::TVOID) {
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


void StubAnalyzer::AnalyzeVariable(SourceLine* line, 
  vector<StubInOutVariable*>& Storage) 
{
  if (line->SubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  StubParameter::Type variableType = TokenToType(line->SubStrings[2]);
  if (!StubParameter::IsValidShaderValueType(variableType)) {
    ERR("line %d: Invalid type");
    return;
  }

  StubInOutVariable* parameter = new StubInOutVariable();
  parameter->type = StubParameter::ToShaderValueType(variableType);
  parameter->name = line->SubStrings[3].ToString();
  Storage.push_back(parameter);
}


void StubAnalyzer::AnalyzeGlobal(SourceLine* line) {
  if (line->SubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  StubParameter::Type declaredType = TokenToType(line->SubStrings[2]);
  SubString& name = line->SubStrings[3];
 
  /// Global sampler
  if (declaredType == StubParameter::Type::SAMPLER2D) {
    int usage = 
      EnumMapperA::GetEnumFromString(GlobalSamplerMapper, name.Begin, name.Length);
    if (usage < 0) {
      ERR("line %d: Unrecognized global sampler '%s'.", mCurrentLineNumber,
        name.ToString().c_str());
      return;
    }
    StubGlobalSampler* globalSampler = new StubGlobalSampler();
    globalSampler->name = name.ToString();
    globalSampler->usage = (GlobalSamplerUsage)usage;
    globalSampler->isMultiSampler = (line->SubStrings[2].Token == TOKEN_sampler2DMS);
    globalSampler->isShadow = (line->SubStrings[2].Token == TOKEN_sampler2DShadow);
    mGlobalSamplers.push_back(globalSampler);
    return;
  }

  /// Global uniform
  if (!StubParameter::IsValidShaderValueType(declaredType)) {
    ERR("line %d: Invalid uniform type'.", mCurrentLineNumber);
    return;
  }

  int usage =
    EnumMapperA::GetEnumFromString(GlobalUniformMapper, name.Begin, name.Length);
  if (usage < 0) {
    ERR("line %d: Unrecognized global uniform '%s'.", mCurrentLineNumber,
      name.ToString().c_str());
    return;
  }

  ShaderValueType shaderType = StubParameter::ToShaderValueType(declaredType);
  ShaderValueType expectedType = GlobalUniformTypes[usage];
  if (shaderType != expectedType) {
    ERR("line %d: wrong type for global uniform '%s'.", mCurrentLineNumber,
        name.ToString().c_str());
    return;
  }

  StubGlobalUniform* globalUniform = new StubGlobalUniform();
  globalUniform->name = name.ToString();
  globalUniform->type = shaderType;
  globalUniform->usage = (GlobalUniformUsage)usage;
  mGlobalUniforms.push_back(globalUniform);
}


StubParameter::Type StubAnalyzer::TokenToType(const SubString& subStr) {
  switch (subStr.Token) {
    case TOKEN_void:            return StubParameter::Type::TVOID;
    case TOKEN_float:           return StubParameter::Type::FLOAT;
    case TOKEN_vec2:            return StubParameter::Type::VEC2;
    case TOKEN_vec3:            return StubParameter::Type::VEC3;
    case TOKEN_vec4:            return StubParameter::Type::VEC4;
    case TOKEN_mat4:            return StubParameter::Type::MATRIX44;
    case TOKEN_sampler2D:       return StubParameter::Type::SAMPLER2D;
    case TOKEN_sampler2DMS:     return StubParameter::Type::SAMPLER2D;
    case TOKEN_sampler2DShadow: return StubParameter::Type::SAMPLER2D;
    default:
      ERR("line %d: Wrong type '%s'", mCurrentLineNumber, subStr.ToString().c_str());
      return StubParameter::Type::NONE;
  }
}
