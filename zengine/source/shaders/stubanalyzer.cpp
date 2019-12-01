#include "stubanalyzer.h"

OWNERSHIP StubMetadata* StubAnalyzer::FromText(const char* stubSource) {
  const StubAnalyzer analyzer(stubSource);

  if (analyzer.mName.empty()) {
    ERR("Shader stub has no name.");
    WARN("source:\n%s", stubSource);
    return nullptr;
  }

  return new StubMetadata(analyzer.mName, analyzer.mReturnType,
                          analyzer.mStrippedSource, analyzer.mParameters,
                          analyzer.mGlobalUniforms, analyzer.mGlobalSamplers,
                          analyzer.mInputs, analyzer.mOutputs);
}

StubAnalyzer::StubAnalyzer(const char* stubSource)
  : mCurrentLineNumber(-1)
    , mReturnType(StubParameter::Type::TVOID) {
  vector<SourceLine> lines = SplitToWords(stubSource);
  for (const SourceLine& line : lines) {
    mCurrentLineNumber = line.mLineNumber;
    if (line.mSubStrings[0].mToken == ShaderTokenEnum::TOKEN_COLON) {
      AnalyzeMetadataLine(line);
    }
    else {
      /// TODO: use string builder
      mStrippedSource = mStrippedSource + string(line.mEntireLine) + '\n';
    }
  }
}

void StubAnalyzer::AnalyzeMetadataLine(const SourceLine& line) {
  if (line.mSubStrings.size() < 2) {
    ERR("line %d: Line empty.", mCurrentLineNumber);
    return;
  }

  switch (line.mSubStrings[1].mToken) {
  case ShaderTokenEnum::TOKEN_name:
    AnalyzeName(line);
    break;
  case ShaderTokenEnum::TOKEN_returns:
    AnalyzeReturns(line);
    break;
  case ShaderTokenEnum::TOKEN_param:
    AnalyzeParam(line);
    break;
  case ShaderTokenEnum::TOKEN_global:
    AnalyzeGlobal(line);
    break;
  case ShaderTokenEnum::TOKEN_input:
    AnalyzeVariable(line, mInputs);
    break;
  case ShaderTokenEnum::TOKEN_output:
    AnalyzeVariable(line, mOutputs);
    break;
  default:
    ERR("line %d: unknown metadata type '%s'.", mCurrentLineNumber,
        string(line.mSubStrings[1].mStringView).c_str());
    break;
  }
}

void StubAnalyzer::AnalyzeName(const SourceLine& line) {
  if (line.mSubStrings.size() != 3 ||
    line.mSubStrings[2].mToken != ShaderTokenEnum::TOKEN_STRING) {
    ERR("line %d: Wrong syntax, use ':name \"<name>\"'", mCurrentLineNumber);
    return;
  }

  mName = line.mSubStrings[2].mStringView;
}

void StubAnalyzer::AnalyzeReturns(const SourceLine& line) {
  if (line.mSubStrings.size() != 3) {
    ERR("line %d: Wrong syntax, use ':returns [void|float|vec2|vec3|vec4]'",
        mCurrentLineNumber);
    return;
  }

  mReturnType = TokenToType(line.mSubStrings[2]);
}

void StubAnalyzer::AnalyzeParam(const SourceLine& line) {
  if (line.mSubStrings.size() < 4) {
    ERR("line %d: Wrong syntax, use ':param <type> <name>'",
        mCurrentLineNumber);
    return;
  }

  StubParameter* parameter = new StubParameter();
  parameter->mType = TokenToType(line.mSubStrings[2]);
  parameter->mName = line.mSubStrings[3].mStringView;
  mParameters.push_back(parameter);
}


void StubAnalyzer::AnalyzeVariable(const SourceLine& line,
                                   vector<StubInOutVariable*>& Storage) const {
  if (line.mSubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  const StubParameter::Type variableType = TokenToType(line.mSubStrings[2]);
  if (!StubParameter::IsValidValueType(variableType)) {
    ERR("line %d: Invalid type");
    return;
  }

  StubInOutVariable* parameter = new StubInOutVariable();
  parameter->type = StubParameter::ToValueType(variableType);
  parameter->name = line.mSubStrings[3].mStringView;
  Storage.push_back(parameter);
}


void StubAnalyzer::AnalyzeGlobal(const SourceLine& line) {
  if (line.mSubStrings.size() < 4) {
    ERR("line %d: Wrong syntax", mCurrentLineNumber);
    return;
  }

  const StubParameter::Type declaredType = TokenToType(line.mSubStrings[2]);
  const SubString& name = line.mSubStrings[3];

  /// Global sampler
  if (declaredType == StubParameter::Type::SAMPLER2D) {
    GlobalSamplerUsage usage = GlobalSamplerMapper.GetEnumA(name.mStringView);
    if (signed(usage) < 0) {
      ERR("line %d: Unrecognized global sampler '%s'.", mCurrentLineNumber,
          string(name.mStringView).c_str());
      return;
    }
    StubGlobalSampler* globalSampler = new StubGlobalSampler();
    globalSampler->name = name.mStringView;
    globalSampler->usage = usage;
    globalSampler->isMultiSampler =
      (line.mSubStrings[2].mToken == ShaderTokenEnum::TOKEN_sampler2DMS);
    globalSampler->isShadow =
      (line.mSubStrings[2].mToken == ShaderTokenEnum::TOKEN_sampler2DShadow);
    mGlobalSamplers.push_back(globalSampler);
    return;
  }

  /// Global uniform
  if (!StubParameter::IsValidValueType(declaredType)) {
    ERR("line %d: Invalid uniform type'.", mCurrentLineNumber);
    return;
  }

  int usage = int(GlobalUniformMapper.GetEnumA(name.mStringView));
  if (usage < 0) {
    ERR("line %d: Unrecognized global uniform '%s'.", mCurrentLineNumber,
        string(name.mStringView).c_str());
    return;
  }

  const ValueType shaderType = StubParameter::ToValueType(declaredType);
  const ValueType expectedType = GlobalUniformTypes[usage];
  if (shaderType != expectedType) {
    ERR("line %d: wrong type for global uniform '%s'.", mCurrentLineNumber,
        string(name.mStringView).c_str());
    return;
  }

  StubGlobalUniform* globalUniform = new StubGlobalUniform();
  globalUniform->name = name.mStringView;
  globalUniform->type = shaderType;
  globalUniform->usage = GlobalUniformUsage(usage);
  mGlobalUniforms.push_back(globalUniform);
}


StubParameter::Type StubAnalyzer::TokenToType(const SubString& subStr) const {
  switch (subStr.mToken) {
  case ShaderTokenEnum::TOKEN_void: return StubParameter::Type::TVOID;
  case ShaderTokenEnum::TOKEN_float: return StubParameter::Type::FLOAT;
  case ShaderTokenEnum::TOKEN_vec2: return StubParameter::Type::VEC2;
  case ShaderTokenEnum::TOKEN_vec3: return StubParameter::Type::VEC3;
  case ShaderTokenEnum::TOKEN_vec4: return StubParameter::Type::VEC4;
  case ShaderTokenEnum::TOKEN_mat4: return StubParameter::Type::MATRIX44;
  case ShaderTokenEnum::TOKEN_sampler2D: return StubParameter::Type::SAMPLER2D;
  case ShaderTokenEnum::TOKEN_sampler2DMS: return StubParameter::Type::SAMPLER2D;
  case ShaderTokenEnum::TOKEN_sampler2DShadow: return StubParameter::Type::SAMPLER2D;
  case ShaderTokenEnum::TOKEN_image2D: return StubParameter::Type::IMAGE2D;
  case ShaderTokenEnum::TOKEN_buffer: return StubParameter::Type::BUFFER;
  default:
    ERR("line %d: Wrong type '%s'", mCurrentLineNumber,
        string(subStr.mStringView).c_str());
    return StubParameter::Type::NONE;
  }
}
