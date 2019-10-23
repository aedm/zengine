#pragma once

#include <include/shaders/stubnode.h>
#include "../shaders/shaderTokenizer.h"

/// TODO: Remove this.
using namespace Shaders;

class StubAnalyzer {
public:
  /// Run analysis for a shader in stub source format.
  /// Returns user-defined metadata extracted from source.
  static OWNERSHIP StubMetadata*	FromText(const char* stubSource);

private:
  StubAnalyzer(const char* stubSource);

  void AnalyzeMetadataLine(SourceLine* line);

  void AnalyzeName(SourceLine* line);
  void AnalyzeReturns(SourceLine* line);
  void AnalyzeParam(SourceLine* line);
  void AnalyzeGlobal(SourceLine* line);
  void AnalyzeVariable(SourceLine* line,
                       std::vector<StubInOutVariable*>& storage) const;

  StubParameter::Type TokenToType(const SubString& subStr) const;

  int mCurrentLineNumber;

  /// Builders
  std::string mName;
  std::string mStrippedSource;
  StubParameter::Type mReturnType;
  std::vector<StubParameter*> mParameters;
  std::vector<StubGlobalUniform*> mGlobalUniforms;
  std::vector<StubGlobalSampler*> mGlobalSamplers;
  std::vector<StubInOutVariable*> mInputs;
  std::vector<StubInOutVariable*> mOutputs;
};