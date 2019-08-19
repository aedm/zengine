#pragma once

#include <include/shaders/shadersource.h>
#include <sstream>

class ShaderBuilder {
public:
  /// Generate a shader source
  static shared_ptr<ShaderSource> GenerateFragmentShaderSource(
    const shared_ptr<StubNode>& fragmentStub);

  static shared_ptr<ShaderSource> GenerateVertexShaderSource(
    const shared_ptr<StubNode>& vertexStub);

private:
  ShaderBuilder();

  shared_ptr<ShaderSource> Build();

  /// How to reference a certain Node dependency within GLSL code? 
  /// Stubs translate to a function call and a variable to store its return value.
  struct StubReference {
    StubReference(StubParameter::Type type);

    /// The variable name in the main function
    string mVariableName;

    /// Function name for stubs
    string mFunctionName;

    /// Stub return type
    const StubParameter::Type mType;
  };

  /// Non-stub Nodes translate to a uniform/sampler value
  struct ValueReference {
    /// Generated uniform/sampler name
    string mName;

    /// Value type
    StubParameter::Type mType;
  };

  /// Inputs and output of the shader stage
  struct InterfaceVariable {
    InterfaceVariable(ValueType type, const string& name, int layout = -1);

    /// Variable type
    ValueType mType;

    /// Variable name
    string mName;

    /// Output layout number for G-Buffers
    int mLayout;
  };

  /// Generates function and variable names for stub calls
  void GenerateStubNames();

  /// Creates topological order of dependency tree
  void CollectDependencies(const shared_ptr<Node>& root);

  /// Traverses stub graph for a shader stage, called only by CollectDependencies
  void TraverseDependencies(const shared_ptr<Node>& root, 
    set<shared_ptr<Node>>& visitedNodes);

  /// Generates function and variable names
  void GenerateNames();

  /// Collect uniforms and samplers
  void AddGlobalsToDependencies();
  void CollectInputsAndOutputs(const shared_ptr<Node>& node);
  void AddLocalsToDependencies();

  /// Generate source
  void GenerateSource();
  void GenerateSourceHeader();
  void GenerateInputInterface();
  void GenerateSourceFunctions();
  void GenerateSourceMain();

  static const string& GetValueTypeString(ValueType type);
  static const string& GetParamTypeString(StubParameter::Type type,
    bool isMultiSampler = false, bool isShadow = false);

  /// Topologic order of dependencies
  vector<shared_ptr<Node>> mDependencies;

  /// References of stub nodes
  map<shared_ptr<Node>, shared_ptr<StubReference>> mStubMap;

  /// Stader stage type
  ShaderType mShaderType = ShaderType::UNKNOWN;

  /// Root stub node
  shared_ptr<StubNode> mRootStubNode;

  /// Generated source code
  stringstream mSourceStream;

  /// Things that need to be #define'd at the beginning of the shader code
  vector<string> mDefines;

  /// Inputs and outputs of the shader stage
  map<string, shared_ptr<InterfaceVariable>> mInputsMap;
  vector<shared_ptr<InterfaceVariable>> mInputs;
  vector<shared_ptr<InterfaceVariable>> mOutputs;

  /// References of uniform/smapler nodes
  map<shared_ptr<Node>, shared_ptr<ValueReference>> mUniformMap;
  map<shared_ptr<Node>, shared_ptr<ValueReference>> mSamplerMap;
  set<GlobalUniformUsage> mUsedGlobalUniforms;
  set<GlobalSamplerUsage> mUsedGlobalSamplers;

  /// Metadata
  vector<ShaderSource::Uniform> mUniforms;
  vector<ShaderSource::Sampler> mSamplers;
};