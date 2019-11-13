#pragma once

#include <include/shaders/stubnode.h>
#include "../shaders/shaderTokenizer.h"
#include <string>
#include <vector>

using std::string;

/// TODO: Remove this.
using namespace Shaders;

class StubAnalyzer {
public:
  /// Run analysis for a shader in stub source format.
  /// Returns user-defined metadata extracted from source.
  static OWNERSHIP StubMetadata*	FromText(const char* stubSource);

private:
  StubAnalyzer(const char* stubSource);

  void AnalyzeMetadataLine(const SourceLine& line);

  void AnalyzeName(const SourceLine& line);
  void AnalyzeReturns(const SourceLine& line);
  void AnalyzeParam(const SourceLine& line);
  void AnalyzeGlobal(const SourceLine& line);
  void AnalyzeVariable(const SourceLine& line,
                       std::vector<StubInOutVariable*>& storage) const;

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