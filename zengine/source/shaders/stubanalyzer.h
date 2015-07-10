#pragma once

#include <include/shaders/stubnode.h>
#include "../shaders/shaderTokenizer.h"

/// TODO: Remove this.
using namespace Shaders;

class StubAnalyzer {
public:
  /// Run analysis for a shader in stub source format.
  /// Returns user-defined metadata extraced from source.
  static OWNERSHIP ShaderStubMetadata*	FromText(const char* stubSource);

private:
  StubAnalyzer(const char* stubSource);

  void AnalyzeMetadataLine(SourceLine* line);

  void AnalyzeName(SourceLine* line);
  void AnalyzeReturns(SourceLine* line);
  void AnalyzeParam(SourceLine* line);
  void AnalyzeGlobal(SourceLine* line);
  void AnalyzeVariable(SourceLine* line,
                       vector<ShaderStubVariable*>& storage);

  NodeType TokenToType(const SubString& subStr);

  int mCurrentLineNumber;

  /// Builders
  string* mName;
  string mStrippedSource;
  NodeType mReturnType;
  vector<ShaderStubParameter*> mParameters;
  vector<ShaderStubGlobal*> mGlobals;
  vector<ShaderStubVariable*> mInputs;
  vector<ShaderStubVariable*> mOutputs;
};