#pragma once

#include <include/shaders/shadersource.h>
#include <sstream>

class ShaderBuilder {
public:
  static std::shared_ptr<ShaderSource> FromStubs(
    const std::shared_ptr<StubNode>& vertexStub, const std::shared_ptr<StubNode>& fragmentStub);

private:
  ShaderBuilder(const std::shared_ptr<StubNode>& vertexStub,
    const std::shared_ptr<StubNode>& fragmentStub);

  std::shared_ptr<ShaderSource> MakeShaderSource();

  /// How to reference a certain Node dependency within GLSL code? 
  /// Stubs translate to a function call and a variable to store its return value.
  struct StubReference {
    StubReference(StubParameter::Type type);

    /// The variable name in the main function
    std::string mVariableName;

    /// Function name for stubs
    std::string mFunctionName;

    /// Stub return type
    const StubParameter::Type mType;
  };

  /// Non-stub Nodes translate to a uniform/sampler value
  struct ValueReference {
    /// Generated uniform/sampler name
    std::string mName;

    /// Value type
    StubParameter::Type mType = StubParameter::Type::TVOID;
  };

  /// Inputs and output of the shader stage
  struct InterfaceVariable {
    InterfaceVariable(ValueType type, std::string name, int layout = -1);

    /// Variable type
    ValueType mType;

    /// Variable name
    std::string mName;

    /// Output layout number for G-Buffers
    int mLayout;
  };

  /// Data for a single shader stage, eg. vertex or fragment shader
  struct ShaderStage {
    ShaderStage(bool isVertexShader);

    /// Generates function and variable names for stub calls
    void GenerateStubNames();

    /// Topologic order of dependencies
    std::vector<std::shared_ptr<Node>> mDependencies;

    /// References of stub nodes
    std::map<std::shared_ptr<Node>, std::shared_ptr<StubReference>> mStubMap;

    /// Stader stage type
    const bool mIsVertexShader;

    /// Things that need to be #define'd at the beginning of the shader code
    std::vector<std::string> mDefines;

    /// Generated source code
    std::stringstream mSourceStream;

    /// Inputs and outputs of the shader stage
    std::map<std::string, std::shared_ptr<InterfaceVariable>> mInputsMap;
    std::vector<std::shared_ptr<InterfaceVariable>> mInputs;
    std::vector<std::shared_ptr<InterfaceVariable>> mOutputs;
  };

  /// Creates topological order of dependency tree
  void CollectDependencies(const std::shared_ptr<Node>& root, ShaderStage* shaderStage);

  /// Traverses stub graph for a shader stage, called only by CollectDependencies
  void TraverseDependencies(const std::shared_ptr<Node>& root,
    ShaderBuilder::ShaderStage* shaderStage, std::set<std::shared_ptr<Node>>& visitedNodes);

  /// Generates function and variable names
  void GenerateNames();

  /// Collect uniforms and samplers
  void AddGlobalsToDependencies(ShaderStage* shaderStage);
  void CollectInputsAndOutputs(const std::shared_ptr<Node>& node, ShaderStage* shaderStage) const;
  void AddLocalsToDependencies();

  /// Generate source
  void GenerateSource(ShaderStage* shaderStage);
  void GenerateSourceHeader(ShaderStage* shaderStage);
  static void GenerateInputInterface(ShaderStage* shaderStage);
  void GenerateSourceFunctions(ShaderStage* shaderStage);
  void GenerateSourceMain(ShaderStage* shaderStage) const;

  static const std::string& GetValueTypeString(ValueType type);
  static const std::string& GetParamTypeString(StubParameter::Type type,
    bool isMultiSampler = false, bool isShadow = false);

  ShaderStage mVertexStage;
  ShaderStage mFragmentStage;

  /// References of uniform/smapler nodes
  std::map<std::shared_ptr<Node>, std::shared_ptr<ValueReference>> mUniformMap;
  std::map<std::shared_ptr<Node>, std::shared_ptr<ValueReference>> mSamplerMap;
  std::map<std::shared_ptr<Node>, std::shared_ptr<ValueReference>> mBufferMap;
  std::set<GlobalUniformUsage> mUsedGlobalUniforms;
  std::set<GlobalSamplerUsage> mUsedGlobalSamplers;

  /// Metadata
  std::vector<ShaderSource::Uniform> mUniforms;
  std::vector<ShaderSource::Sampler> mSamplers;
  std::vector<ShaderSource::NamedResource> mSSBOs;
};