#pragma once

#include <include/shaders/shadersource.h>
#include <sstream>

class ShaderBuilder {
public:
  static shared_ptr<ShaderSource> FromStubs(
    const shared_ptr<StubNode>& vertexStub, const shared_ptr<StubNode>& fragmentStub);

private:
  ShaderBuilder(const shared_ptr<StubNode>& vertexStub, 
    const shared_ptr<StubNode>& fragmentStub);

  shared_ptr<ShaderSource> MakeShaderSource();

  /// How to reference a certain Node dependency within GLSL code? 
  /// Stubs translate to a function call and a variable to store its return value.
  struct StubReference {
    StubReference(ValueType type);

    /// The variable name in the main function
    string mVariableName;

    /// Function name for stubs
    string mFunctionName;

    /// Stub return type
    const ValueType mType;
  };

  /// Non-stub Nodes translate to a uniform/sampler value
  struct ValueReference {
    /// Generated uniform/sampler name
    string mName;

    /// Value type
    ValueType mType;
  };

  /// Inputs and output of the shader stage
  struct InOutVariable {
    InOutVariable(ValueType type, const string& name, int layout = -1);

    /// Variable type
    ValueType mType;

    /// Variable name
    string mName;

    /// Output layout number for G-Buffers
    int mLayout;
  };

  /// Data for a single shader stage, eg. vertex or fragment shader
  struct ShaderStage {
    ShaderStage(bool isVertexShader);

    /// Generates function and variable names for stub calls
    void GenerateStubNames();

    /// Topologic order of dependencies
    vector<shared_ptr<Node>> mDependencies;

    /// References of stub nodes
    map<shared_ptr<Node>, shared_ptr<StubReference>> mStubMap;

    /// Stader stage type
    const bool mIsVertexShader;

    /// Things that need to be #define'd at the beginning of the shader code
    vector<string> mDefines;

    /// Generated source code
    stringstream mSourceStream;

    /// Inputs and outputs of the shader stage
    map<string, shared_ptr<InOutVariable>> mInputsMap;
    vector<shared_ptr<InOutVariable>> mInputs;
    vector<shared_ptr<InOutVariable>> mOutputs;
  };

  /// Creates topological order of dependency tree
  void CollectDependencies(const shared_ptr<Node>& root, ShaderStage* shaderStage);

  /// Traverses stub graph for a shader stage, called only by CollectDependencies
  void TraverseDependencies(const shared_ptr<Node>& root, 
    ShaderBuilder::ShaderStage* shaderStage, set<shared_ptr<Node>>& visitedNodes);

  /// Generates function and variable names
  void GenerateNames();

  /// Generate metadata
  void AddGlobalsToDependencies(ShaderStage* shaderStage);
  void CollectInputsAndOutputs(const shared_ptr<Node>& node, ShaderStage* shaderStage);
  void AddLocalsToDependencies();

  /// Generate source
  void GenerateSource(ShaderStage* shaderStage);
  void GenerateSourceHeader(ShaderStage* shaderStage);
  void GenerateSourceFunctions(ShaderStage* shaderStage);
  void GenerateSourceMain(ShaderStage* shaderStage);

  static const string& GetTypeString(ValueType type, bool isMultiSampler = false,
                                     bool isShadow = false);

  ShaderStage mVertexStage;
  ShaderStage mFragmentStage;

  /// References of uniform/smapler nodes
  map<shared_ptr<Node>, shared_ptr<ValueReference>> mUniformMap;
  map<shared_ptr<Node>, shared_ptr<ValueReference>> mSamplerMap;
  set<ShaderGlobalType> mUsedGlobals;

  /// Metadata
  vector<ShaderSource::Uniform> mUniforms;
  vector<ShaderSource::Sampler> mSamplers;
};