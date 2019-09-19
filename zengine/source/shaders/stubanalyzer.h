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
                       vector<StubInOutVariable*>& storage);

  StubParameter::Type TokenToType(const SubString& subStr) const;

  int mCurrentLineNumber;

  /// Builders
  string mName;
  string mStrippedSource;
  StubParameter::Type mReturnType;
  vector<StubParameter*> mParameters;
  vector<StubGlobalUniform*> mGlobalUniforms;
  vector<StubGlobalSampler*> mGlobalSamplers;
  vector<StubInOutVariable*> mInputs;
  vector<StubInOutVariable*> mOutputs;
};