#include "shaderbuilder.h"
#include <include/shaders/enginestubs.h>
#include <include/nodes/texturenode.h>
#include <exception>

shared_ptr<ShaderSource> ShaderBuilder::GenerateFragmentShaderSource(
  const shared_ptr<StubNode>& fragmentStub)
{
  ASSERT(fragmentStub != nullptr);
  ShaderBuilder shaderBuilder;
  shaderBuilder.mRootStubNode = fragmentStub;
  shaderBuilder.mShaderType = ShaderType::FRAGMENT;
  return shaderBuilder.Build();
}

shared_ptr<ShaderSource> ShaderBuilder::GenerateVertexShaderSource(
  const shared_ptr<StubNode>& vertexStub)
{
  ASSERT(vertexStub != nullptr);
  ShaderBuilder shaderBuilder;
  shaderBuilder.mRootStubNode = vertexStub;
  shaderBuilder.mShaderType = ShaderType::VERTEX;
  return shaderBuilder.Build();
}


ShaderBuilder::InterfaceVariable::InterfaceVariable(ValueType type, const string& name, 
  int layout)
  : mName(name)
  , mType(type)
  , mLayout(layout) {}


ShaderBuilder::ShaderBuilder() {}

shared_ptr<ShaderSource> ShaderBuilder::Build() {
  StubMetadata* stubMeta = mRootStubNode->GetStubMetadata();
  if (stubMeta == nullptr) {
    ERR("Stub has no metadata.");
    return nullptr;
  }

  /// Collects node dependencies
  INFO("Analyzing stub: '%s'", stubMeta->name.c_str());

  try {
    /// Traverse the dependency tree
    CollectDependencies(mRootStubNode);

    /// Add globals to uniforms and samplers
    AddGlobalsToDependencies();

    /// Generate shader varaible names for collected dependency nodes
    GenerateNames();

    /// Add locals to uniforms and samplers
    AddLocalsToDependencies();

    INFO("Building shader source for '%s'...", stubMeta->name.c_str());
    GenerateSource();
  }
  catch (...) {
    ERR("Shader source creation failed");
  }

  return make_shared<ShaderSource>(mUniforms, mSamplers, mSourceStream.str());
}

void ShaderBuilder::CollectInputsAndOutputs(
  const shared_ptr<Node>& node)
{
  shared_ptr<StubNode> stub = PointerCast<StubNode>(node);

  StubMetadata* stubMeta = stub->GetStubMetadata();
  if (stubMeta == nullptr) {
    ERR("Can't build shader source.");
    throw exception();
  }

  /// Inputs
  for (auto input : stubMeta->inputs) {
    /// TODO: check whether input types match
    if (mInputsMap.find(input->name) == mInputsMap.end()) {
      mInputsMap[input->name] = make_shared<InterfaceVariable>(input->type, input->name);
    }
  }

  /// Outputs
  for (auto output : stubMeta->outputs) {
    int layout = -1;
    if (output->name == "GBufferTargetA") layout = 0;
    else if (output->name == "GBufferTargetB") layout = 1;
    else if (output->name == "GBufferTargetC") layout = 2;
    else if (output->name == "GBufferTargetD") layout = 3;
    mOutputs.push_back(
      make_shared<InterfaceVariable>(output->type, output->name, layout));
  }
}

StubParameter::Type NodeToParamType(const shared_ptr<Node>& node) {
  if (IsPointerOf<ValueNode<float>>(node)) return StubParameter::Type::FLOAT;
  if (IsPointerOf<ValueNode<Vec2>>(node)) return StubParameter::Type::VEC2;
  if (IsPointerOf<ValueNode<Vec3>>(node)) return StubParameter::Type::VEC3;
  if (IsPointerOf<ValueNode<Vec4>>(node)) return StubParameter::Type::VEC4;
  if (IsPointerOf<ValueNode<Matrix>>(node)) return StubParameter::Type::MATRIX44;
  if (IsPointerOf<ValueNode<shared_ptr<Texture>>>(node)) return StubParameter::Type::SAMPLER2D;
  SHOULD_NOT_HAPPEN;
  return StubParameter::Type::NONE;
}

void ShaderBuilder::TraverseDependencies(const shared_ptr<Node>& root,
  set<shared_ptr<Node>>& visitedNodes)
{
  visitedNodes.insert(root);

  if (IsPointerOf<StubNode>(root)) {
    /// Assigns empty stub reference
    auto stubNode = PointerCast<StubNode>(root);
    stubNode->Update();

    StubParameter::Type returnType = stubNode->GetStubMetadata()->returnType;
    mStubMap[root] = make_shared<StubReference>(returnType);

    for (auto& slotPair : stubNode->mParameterSlotMap) {
      shared_ptr<Node> node = slotPair.second->GetReferencedNode();
      if (node == nullptr) {
        WARN("Incomplete shader graph.");
        throw exception();
      }
      if (slotPair.first->mType == StubParameter::Type::SAMPLER2D) {
        auto& textureNode = PointerCast<TextureNode>(node);
        ASSERT(textureNode);
        if (textureNode->Get() != nullptr) {
          mDefines.push_back(slotPair.first->mName + "_CONNECTED");
        }
      }
      if (visitedNodes.find(node) == visitedNodes.end()) {
        TraverseDependencies(node, visitedNodes);
      }
    }
  }
  else {
    auto ref = make_shared<ValueReference>();
    StubParameter::Type type = NodeToParamType(root);
    ASSERT(type != StubParameter::Type::NONE);
    ref->mType = type;
    if (type == StubParameter::Type::SAMPLER2D) {
      if (mSamplerMap.find(root) == mSamplerMap.end()) {
        mSamplerMap[root] = ref;
      }
    }
    else {
      if (mUniformMap.find(root) == mUniformMap.end()) {
        mUniformMap[root] = ref;
      }
    }
  }

  mDependencies.push_back(root);
}


void ShaderBuilder::CollectDependencies(const shared_ptr<Node>& root) {
  shared_ptr<StubNode> uberShader = TheEngineStubs->GetStub("uber");
  TraverseDependencies(uberShader, set<shared_ptr<Node>>());
  TraverseDependencies(root, set<shared_ptr<Node>>());
}


void ShaderBuilder::GenerateStubNames() {
  int stubIndex = 0;
  for (const auto& node : mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubReference> stubReference = mStubMap.at(node);
      ++stubIndex;

      /// The referenced name becomes the variable name within the "main" function
      stringstream functionName;
      functionName << "_func_" << stubIndex;
      stubReference->mFunctionName = functionName.str();

      stringstream variableName;
      variableName << "_var_" << stubIndex;
      stubReference->mVariableName = variableName.str();
    }
  }
}


void ShaderBuilder::GenerateNames() {
  /// Generate function and variable names for stubs
  GenerateStubNames();

  /// Generate names for uniforms
  int uniformIndex = 0;
  for (auto& it : mUniformMap) {
    stringstream uniformName;
    uniformName << "_uniform_" << ++uniformIndex;
    it.second->mName = uniformName.str();
  }

  /// Generate names for samplers
  int samplerIndex = 0;
  for (auto& it : mSamplerMap) {
    stringstream samplerName;
    samplerName << "_sampler_" << ++samplerIndex;
    it.second->mName = samplerName.str();
  }
}


void ShaderBuilder::AddGlobalsToDependencies() {
  for (const auto& node : mDependencies) {
    if (!IsPointerOf<StubNode>(node)) continue;

    auto stub = PointerCast<StubNode>(node);
    StubMetadata* stubMeta = stub->GetStubMetadata();
    if (stubMeta == nullptr) {
      ERR("Can't build shader source.");
      throw exception();
    }

    for (StubGlobalUniform* global : stubMeta->globalUniforms) {
      if (mUsedGlobalUniforms.find(global->usage) == mUsedGlobalUniforms.end()) {
        mUsedGlobalUniforms.insert(global->usage);
        mUniforms.emplace_back(global->name, nullptr, global->usage, global->type);
      }
    }

    for (StubGlobalSampler* global : stubMeta->globalSamplers) {
      if (mUsedGlobalSamplers.find(global->usage) == mUsedGlobalSamplers.end()) {
        mUsedGlobalSamplers.insert(global->usage);
        mSamplers.emplace_back(global->name, nullptr, global->usage,
          global->isMultiSampler, global->isShadow);
      }
    }
  }
}

void ShaderBuilder::AddLocalsToDependencies() {
  for (auto& it : mUniformMap) {
    mUniforms.emplace_back(
      it.second->mName, it.first, GlobalUniformUsage::LOCAL, NodeToValueType(it.first));
  }
  for (auto& it : mSamplerMap) {
    mSamplers.emplace_back(
      it.second->mName, it.first, GlobalSamplerUsage::LOCAL, false, false);
  }
}

void ShaderBuilder::GenerateSource() {
  for (const auto& node : mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      CollectInputsAndOutputs(node);
    }
  }

  GenerateSourceHeader();
  GenerateSourceFunctions();
  GenerateSourceMain();
}

void ShaderBuilder::GenerateInputInterface() {
  /// Array for attribute names
  static const char* gVertexAttributeName[] = {
    "aPosition",
    "aTexCoord",
    "aNormal",
    "aTangent",
  };
  
  stringstream& stream = mSourceStream;

  if (mShaderType == ShaderType::VERTEX) {
    for (const auto& var : mInputsMap) {
      WARN("Unnecessary attribute definition in vertex shader: %s", var.first.c_str());
    }
    /// The inputs of the vertex shader is a fixed layout of vertex attributes
    for (UINT i = 0; i < UINT(VertexAttributeUsage::COUNT); i++) {
      ValueType attribValue = VertexAttributeUsageToValueType(VertexAttributeUsage(i));
      stream << "layout(location = " << i << ") in " <<
        GetValueTypeString(attribValue) << ' ' << gVertexAttributeName[i] << ';' << endl;
    }
    return;
  }

  /// Inputs
  for (const auto& var : mInputsMap) {
    stream << "in " << GetValueTypeString(var.second->mType) << ' ' <<
      var.second->mName << ';' << endl;
    mInputs.push_back(var.second);
  }
}


void ShaderBuilder::GenerateSourceHeader() {
  stringstream& stream = mSourceStream;

  stream << "#version 460 core" << endl;
  switch (mShaderType)
  {
  case ShaderType::VERTEX:
    stream << "#define VERTEX_SHADER" << endl;
    break;
  case ShaderType::FRAGMENT:
    stream << "#define FRAGMENT_SHADER" << endl;
    break;
  case ShaderType::COMPUTE:
    stream << "#define COMPUTE_SHADER" << endl;
    break;
  default:
    SHOULD_NOT_HAPPEN;
  }


  GenerateInputInterface();

  /// Outputs
  for (const auto& var : mOutputs) {
    if (var->mLayout >= 0) {
      stream << "layout (location = " << var->mLayout << ") ";
    }
    stream << "out " << GetValueTypeString(var->mType) << " " << var->mName << ";" << 
      endl;
  }

  /// Uniform block
  UINT binding = -1;
  const char* uniformBlockName = nullptr;
  switch (mShaderType)
  {
  case ShaderType::VERTEX:
    binding = 0;
    uniformBlockName = "VertexUniforms";
    break;
  case ShaderType::FRAGMENT:
    binding = 1;
    uniformBlockName = "FragmentUniforms";
    break;
  default:
    binding = 0;
    uniformBlockName = "Uniforms";
    break;
  }
  mSourceStream << "layout(packed, binding=" << binding  << ") uniform " <<
    uniformBlockName << " {" << endl;
  for (const auto& uniform : mUniforms) {
    stream << "  " << GetValueTypeString(uniform.mType) << " " << uniform.mName << 
      ";" << endl;
  }
  stream << "};" << endl;

  /// Samplers
  /// They are opaque types, thus cannot be part of uniform buffers.
  for (auto sampler : mSamplers) {
    stream << "uniform " <<
      GetParamTypeString(StubParameter::Type::SAMPLER2D, sampler.mIsMultiSampler, 
        sampler.mIsShadow) << ' ' << sampler.mName << ';' << endl;
  }

  /// Stub inputs as variables
  for (const auto& node : mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      auto& stubReference = mStubMap.at(node);

      if (stubReference->mType != StubParameter::Type::TVOID) {
        stream << GetParamTypeString(stubReference->mType) << ' ' <<
          stubReference->mVariableName << ";" << endl;
      }
    }
  }

  /// Defines
  for (auto define : mDefines) {
    stream << "#define " << define << endl;
  }
}

void ShaderBuilder::GenerateSourceFunctions() {
  stringstream& stream = mSourceStream;

  for (const auto& node : mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      stream << endl;

      /// Define :params
      for (StubParameter* param : stubMeta->parameters) {
        Slot* slot = stub->GetSlotByParameter(param);
        auto paramNode = slot->GetReferencedNode();
        if (paramNode == nullptr) {
          /// Node not connected to param
          ERR("Parameter not connected");
          throw exception();
        }
        if (param->mType == StubParameter::Type::SAMPLER2D) {
          /// Parameter is a texture
          auto& valueReference = mSamplerMap.at(paramNode);
          stream << "#define " << param->mName << ' ' << valueReference->mName << endl;
        }
        else if (IsPointerOf<StubNode>(paramNode)) {
          /// Parameter is the result of a former function call
          auto& stubReference = mStubMap.at(paramNode);
          stream << "#define " << param->mName << ' ' << stubReference->mVariableName <<
            endl;
        }
        else {
          /// Parameter is a uniform
          auto& valueReference = mUniformMap.at(paramNode);
          stream << "#define " << param->mName << ' ' << valueReference->mName << endl;
        }
      }

      /// Define SHADER function signature
      auto stubReference = mStubMap.at(node);

      stream << "#define SHADER " << GetParamTypeString(stubMeta->returnType) <<
        ' ' << stubReference->mFunctionName << "()" << endl;

      /// Main shader code
      stream << stubMeta->strippedSource;

      /// Undefine SHADER macro and samplers
      stream << "#undef SHADER" << endl;
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        StubParameter* param = stubMeta->parameters[i];
        stream << "#undef " << param->mName << endl;
      }
    }
  }
}


void ShaderBuilder::GenerateSourceMain() {
  auto& stream = mSourceStream;

  stream << endl;
  stream << "void main() {" << endl;
  for (const auto& node : mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      auto& stubReference = mStubMap.at(node);

      stream << "  ";
      if (stubReference->mType != StubParameter::Type::TVOID) {
        stream << stubReference->mVariableName << " = ";
      }
      stream << stubReference->mFunctionName << "(";
      stream << ");" << endl;
    }
  }
  stream << "}" << endl;
}


const string& ShaderBuilder::GetValueTypeString(ValueType type) {
  static const string sfloat("float");
  static const string svec2("vec2");
  static const string svec3("vec3");
  static const string svec4("vec4");
  static const string suint("uint");
  static const string smatrix44("mat4");
  static const string sinvalid("INVALID TYPE");

  switch (type) {
  case ValueType::FLOAT:      return sfloat;
  case ValueType::VEC2:      return svec2;
  case ValueType::VEC3:      return svec3;
  case ValueType::VEC4:      return svec4;
  case ValueType::MATRIX44:  return smatrix44;
  }

  SHOULD_NOT_HAPPEN;
  return sinvalid;
}

const string& ShaderBuilder::GetParamTypeString(StubParameter::Type type,
  bool isMultiSampler, bool isShadow)
{
  static const string sfloat("float");
  static const string svec2("vec2");
  static const string svec3("vec3");
  static const string svec4("vec4");
  static const string smatrix44("mat4");
  static const string ssampler2d("sampler2D");
  static const string ssampler2dms("sampler2DMS");
  static const string ssampler2dshadow("sampler2DShadow");
  static const string svoid("void");
  static const string serror("UNKNOWN_TYPE");

  if (isMultiSampler) return ssampler2dms;
  if (isShadow) return ssampler2dshadow;

  switch (type) {
  case StubParameter::Type::FLOAT:      return sfloat;
  case StubParameter::Type::VEC2:       return svec2;
  case StubParameter::Type::VEC3:       return svec3;
  case StubParameter::Type::VEC4:       return svec4;
  case StubParameter::Type::MATRIX44:   return smatrix44;
  case StubParameter::Type::SAMPLER2D:  return ssampler2d;
  case StubParameter::Type::TVOID:      return svoid;
  default:
    ERR("Unhandled type: %d", type);
    return serror;
  }
}

ShaderBuilder::StubReference::StubReference(StubParameter::Type type)
  :mType(type) {}
