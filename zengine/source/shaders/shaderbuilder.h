#pragma once

#include <include/shaders/shadersource.h>
#include <sstream>

class ShaderBuilder {
public:
  static OWNERSHIP ShaderSource* FromStubs(StubNode* vertexStub, StubNode* fragmentStub);

  OWNERSHIP ShaderSource* MakeShaderSource();

private:
  ShaderBuilder(StubNode* vertexStub, StubNode* fragmentStub);
  ~ShaderBuilder();

  /// How to reference a certain Node dependency within GLSL code? 
  /// Stubs translate to a function call and a variable to store its return value.
  struct StubReference {
    /// The variable name in the main function
    string mVariableName;

    /// Function name for stubs
    string mFunctionName;

    /// Stub return type
    NodeType mType;
  };

  /// Non-stub Nodes translate to a uniform/sampler value
  struct ValueReference {
    /// Generated uniform/sampler name
    string mName;

    /// Value type
    NodeType mType;
  };

  /// Inputs and output of the shader stage
  struct InOutVariable {
    InOutVariable(NodeType type, const string& name, int layout = -1);

    /// Variable type
    NodeType mType;

    /// Variable name
    string mName;

    /// Output layout number for G-Buffers
    int mLayout;
  };

  /// Data for a single shader stage, eg. vertex or fragment shader
  struct ShaderStage {
    ShaderStage();

    /// Generates function and variable names for stub calls
    void GenerateStubNames();

    /// Topologic order of dependencies
    vector<Node*> mDependencies;

    /// References of stub nodes
    map<Node*, shared_ptr<StubReference>> mStubMap;

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
  void CollectDependencies(Node* root, ShaderStage* shaderStage);

  /// Traverses stub graph for a shader stage, called only by CollectDependencies
  void TraverseDependencies(Node* root, ShaderBuilder::ShaderStage* shaderStage,
                            set<Node*>& visitedNodes);

  /// Generates function and variable names
  void GenerateNames();

  /// Generate metadata
  void AddGlobalsToDependencies(ShaderStage* shaderStage);
  void CollectInputsAndOutputs(Node* node, ShaderStage* shaderStage);
  void AddLocalsToDependencies();

  /// Generate source
  void GenerateSource(ShaderStage* shaderStage);
  void GenerateSourceHeader(ShaderStage* shaderStage);
  void GenerateSourceFunctions(ShaderStage* shaderStage);
  void GenerateSourceMain(ShaderStage* shaderStage);

  static const string& GetTypeString(NodeType type, bool isMultiSampler = false, 
                                     bool isShadow = false);

  ShaderStage mVertexStage;
  ShaderStage mFragmentStage;

  /// References of uniform/smapler nodes
  map<Node*, shared_ptr<ValueReference>> mUniformMap;
  map<Node*, shared_ptr<ValueReference>> mSamplerMap;
  set<ShaderGlobalType> mUsedGlobals;

  /// Metadata
  vector<ShaderSource::Uniform> mUniforms;
  vector<ShaderSource::Sampler> mSamplers;
};