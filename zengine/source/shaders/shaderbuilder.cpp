#include "shaderbuilder.h"
#include <include/shaders/enginestubs.h>
#include <include/nodes/texturenode.h>
#include <include/nodes/buffernode.h>
#include <exception>
#include <utility>

shared_ptr<ShaderSource> ShaderBuilder::FromStubs(const shared_ptr<StubNode>& vertexStub,
  const shared_ptr<StubNode>& fragmentStub)
{
  if (vertexStub == nullptr) {
    ERR("vertex stub is nullptr");
    return nullptr;
  }
  if (fragmentStub == nullptr) {
    ERR("fragment stub is nullptr");
    return nullptr;
  }

  ShaderBuilder shaderBuilder(vertexStub, fragmentStub);

  return shaderBuilder.MakeShaderSource();
}


ShaderBuilder::InterfaceVariable::InterfaceVariable(ValueType type, string name, 
  int layout)
  : mType(type)
  , mName(std::move(name))
  , mLayout(layout) {}


ShaderBuilder::ShaderStage::ShaderStage(bool isVertexShader)
  : mIsVertexShader(isVertexShader) {}


ShaderBuilder::ShaderBuilder(const shared_ptr<StubNode>& vertexStub,
  const shared_ptr<StubNode>& fragmentStub)
  : mVertexStage(true)
  , mFragmentStage(false)
{
  StubMetadata* vertexStubMeta = vertexStub->GetStubMetadata();
  if (vertexStubMeta == nullptr) {
    ERR("Vertex stub has no metadata.");
    return;
  }
  StubMetadata* fragmentStubMeta = fragmentStub->GetStubMetadata();
  if (fragmentStubMeta == nullptr) {
    ERR("Fragment stub has no metadata.");
    return;
  }

  /// Collects node dependencies
  INFO("Analyzing stubs, vs: '%s', fs: '%s' ...", vertexStubMeta->mName.c_str(),
    fragmentStubMeta->mName.c_str());

  try {
    /// Traverse the dependency tree
    CollectDependencies(vertexStub, &mVertexStage);
    CollectDependencies(fragmentStub, &mFragmentStage);

    /// Add globals to uniforms and samplers
    AddGlobalsToDependencies(&mVertexStage);
    AddGlobalsToDependencies(&mFragmentStage);

    /// Generate shader varaible names for collected dependency nodes
    GenerateNames();

    /// Add locals to uniforms and samplers
    AddLocalsToDependencies();

    INFO("Building vertex shader source for '%s'...", vertexStubMeta->mName.c_str());
    GenerateSource(&mVertexStage);

    INFO("Building fragment shader source for '%s'...", fragmentStubMeta->mName.c_str());
    GenerateSource(&mFragmentStage);
  }
  catch (...) {
    ERR("Shader source creation failed");
  }
}

shared_ptr<ShaderSource> ShaderBuilder::MakeShaderSource() {
  return make_shared<ShaderSource>(mUniforms, mSamplers, mSSBOs,
    mVertexStage.mSourceStream.str(),
    mFragmentStage.mSourceStream.str());
}

void ShaderBuilder::CollectInputsAndOutputs(
  const shared_ptr<Node>& node, ShaderStage* shaderStage) const
{
  const shared_ptr<StubNode> stub = PointerCast<StubNode>(node);

  StubMetadata* stubMeta = stub->GetStubMetadata();
  if (stubMeta == nullptr) {
    ERR("Can't build shader source.");
    throw exception();
  }

  /// Inputs
  for (auto input : stubMeta->mInputs) {
    /// TODO: check whether input types match
    if (shaderStage->mInputsMap.find(input->name) == shaderStage->mInputsMap.end()) {
      shaderStage->mInputsMap[input->name] =
        make_shared<InterfaceVariable>(input->type, input->name);
    }
  }

  /// Outputs
  for (auto output : stubMeta->mOutputs) {
    int layout = -1;
    if (output->name == "GBufferTargetA") layout = 0;
    else if (output->name == "GBufferTargetB") layout = 1;
    else if (output->name == "GBufferTargetC") layout = 2;
    else if (output->name == "GBufferTargetD") layout = 3;
    shaderStage->mOutputs.push_back(
      make_shared<InterfaceVariable>(output->type, output->name, layout));
  }
}

StubParameter::Type NodeToParamType(const shared_ptr<Node>& node) {
  if (IsPointerOf<ValueNode<float>>(node)) return StubParameter::Type::FLOAT;
  if (IsPointerOf<ValueNode<Vec2>>(node)) return StubParameter::Type::VEC2;
  if (IsPointerOf<ValueNode<Vec3>>(node)) return StubParameter::Type::VEC3;
  if (IsPointerOf<ValueNode<Vec4>>(node)) return StubParameter::Type::VEC4;
  if (IsPointerOf<ValueNode<Matrix>>(node)) return StubParameter::Type::MATRIX44;
  if (IsPointerOf<ValueNode<shared_ptr<Texture>>>(node)) {
    return StubParameter::Type::SAMPLER2D;
  }
  if (IsPointerOf<BufferNode>(node)) return StubParameter::Type::BUFFER;
  SHOULD_NOT_HAPPEN;
  return StubParameter::Type::NONE;
}

void ShaderBuilder::TraverseDependencies(const shared_ptr<Node>& root,
  ShaderStage* shaderStage, set<shared_ptr<Node>>& visitedNodes)
{
  visitedNodes.insert(root);

  if (IsPointerOf<StubNode>(root)) {
    /// Assigns empty stub reference
    auto stubNode = PointerCast<StubNode>(root);
    stubNode->Update();

    StubParameter::Type returnType = stubNode->GetStubMetadata()->mReturnType;
    shaderStage->mStubMap[root] = make_shared<StubReference>(returnType);

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
          shaderStage->mDefines.push_back(slotPair.first->mName + "_CONNECTED");
        }
      }
      if (visitedNodes.find(node) == visitedNodes.end()) {
        TraverseDependencies(node, shaderStage, visitedNodes);
      }
    }
  }
  else {
    auto ref = make_shared<ValueReference>();
    const StubParameter::Type type = NodeToParamType(root);
    ASSERT(type != StubParameter::Type::NONE);
    ref->mType = type;
    switch (type) {
    case StubParameter::Type::SAMPLER2D:
      if (mSamplerMap.find(root) == mSamplerMap.end()) {
        mSamplerMap[root] = ref;
      }
      break;
    case StubParameter::Type::IMAGE2D:
      NOT_IMPLEMENTED;
      break;
    case StubParameter::Type::BUFFER:
      if (mBufferMap.find(root) == mBufferMap.end()) {
        mBufferMap[root] = ref;
      }
      break;
    default:
      /// Value type, plain uniform
      if (mUniformMap.find(root) == mUniformMap.end()) {
        mUniformMap[root] = ref;
      }
    }
  }

  shaderStage->mDependencies.push_back(root);
}


void ShaderBuilder::CollectDependencies(const shared_ptr<Node>& root,
  ShaderStage* shaderStage)
{
  const shared_ptr<StubNode> uberShader = TheEngineStubs->GetStub("uber");
  set<shared_ptr<Node>> uberShaderDeps;
  TraverseDependencies(uberShader, shaderStage, uberShaderDeps);
  set<shared_ptr<Node>> rootDeps;
  TraverseDependencies(root, shaderStage, rootDeps);
}


void ShaderBuilder::ShaderStage::GenerateStubNames() {
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
  mVertexStage.GenerateStubNames();
  mFragmentStage.GenerateStubNames();

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

  int bufferIndex = 0;
  for (auto& it : mBufferMap) {
    stringstream bufferName;
    bufferName << "_buffer_" << ++bufferIndex;
    it.second->mName = bufferName.str();
  }
}


void ShaderBuilder::AddGlobalsToDependencies(ShaderStage* shaderStage) {
  for (const auto& node : shaderStage->mDependencies) {
    if (!IsPointerOf<StubNode>(node)) continue;

    const auto stub = PointerCast<StubNode>(node);
    StubMetadata* stubMeta = stub->GetStubMetadata();
    if (stubMeta == nullptr) {
      ERR("Can't build shader source.");
      throw exception();
    }

    for (StubGlobalUniform* global : stubMeta->mGlobalUniforms) {
      if (mUsedGlobalUniforms.find(global->usage) == mUsedGlobalUniforms.end()) {
        mUsedGlobalUniforms.insert(global->usage);
        mUniforms.emplace_back(global->name, nullptr, global->usage, global->type);
      }
    }

    for (StubGlobalSampler* global : stubMeta->mGlobalSamplers) {
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
  for (auto& it : mBufferMap) {
    mSSBOs.emplace_back(it.second->mName, it.first);
  }
}

void ShaderBuilder::GenerateSource(ShaderStage* shaderStage) {
  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      CollectInputsAndOutputs(node, shaderStage);
    }
  }

  GenerateSourceHeader(shaderStage);
  GenerateSourceFunctions(shaderStage);
  GenerateSourceMain(shaderStage);
}

void ShaderBuilder::GenerateInputInterface(ShaderStage* shaderStage) {
  /// Array for attribute names
  static const char* gVertexAttributeName[] = {
    "aPosition",
    "aTexCoord",
    "aNormal",
    "aTangent",
  };
  
  stringstream& stream = shaderStage->mSourceStream;

  if (shaderStage->mIsVertexShader) {
    for (const auto& var : shaderStage->mInputsMap) {
      WARN("Unnecessary attribute definition in vertex shader: %s", var.first.c_str());
    }
    /// The inputs of the vertex shader is a fixed layout of vertex attributes
    for (UINT i = 0; i < UINT(VertexAttributeUsage::COUNT); i++) {
      const ValueType attribValue = VertexAttributeUsageToValueType(VertexAttributeUsage(i));
      stream << "layout(location = " << i << ") in " <<
        GetValueTypeString(attribValue) << ' ' << gVertexAttributeName[i] << ';' << endl;
    }
    return;
  }

  /// Inputs
  for (const auto& var : shaderStage->mInputsMap) {
    stream << "in " << GetValueTypeString(var.second->mType) << ' ' <<
      var.second->mName << ';' << endl;
    shaderStage->mInputs.push_back(var.second);
  }
}


void ShaderBuilder::GenerateSourceHeader(ShaderStage* shaderStage) {
  stringstream& stream = shaderStage->mSourceStream;

  stream << "#version 430 core" << endl;
  stream << "#define " <<
    (shaderStage->mIsVertexShader ? "VERTEX_SHADER" : "FRAGMENT_SHADER") << endl;

  GenerateInputInterface(shaderStage);

  /// Outputs
  for (const auto& var : shaderStage->mOutputs) {
    if (var->mLayout >= 0) {
      stream << "layout (location = " << var->mLayout << ") ";
    }
    stream << "out " << GetValueTypeString(var->mType) << " " << var->mName << ";" << 
      endl;
  }

  /// Uniform block
  shaderStage->mSourceStream << "layout(shared) uniform Uniforms {" << endl;
  for (const auto& uniform : mUniforms) {
    stream << "  " << GetValueTypeString(uniform.mType) << " " << uniform.mName << 
      ";" << endl;
  }
  stream << "};" << endl;

  /// Samplers
  /// They are opaque types, thus cannot be part of uniform buffers.
  for (const auto& sampler : mSamplers) {
    stream << "uniform " <<
      GetParamTypeString(StubParameter::Type::SAMPLER2D, sampler.mIsMultiSampler, 
        sampler.mIsShadow) << ' ' << sampler.mName << ';' << endl;
  }

  /// Buffers
  for (const auto& buffer : mSSBOs) {
    stream << "layout(std140) buffer " << buffer.mName << " {" << endl <<
      "  vec4 " << buffer.mName << "_items[];" << endl <<
      "};" << endl;
  }

  /// Stub inputs as variables
  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      const shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      auto& stubReference = shaderStage->mStubMap.at(node);

      if (stubReference->mType != StubParameter::Type::TVOID) {
        stream << GetParamTypeString(stubReference->mType) << ' ' <<
          stubReference->mVariableName << ";" << endl;
      }
    }
  }

  /// Defines
  for (const auto& define : shaderStage->mDefines) {
    stream << "#define " << define << endl;
  }
}

void ShaderBuilder::GenerateSourceFunctions(ShaderStage* shaderStage) {
  stringstream& stream = shaderStage->mSourceStream;

  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      stream << endl;

      /// Define :params
      for (StubParameter* param : stubMeta->mParameters) {
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
        else if (param->mType == StubParameter::Type::BUFFER) {
          /// Parameter is a buffer. The alias should refer to the inner array.
          /// "buffer foo { vec4 foo_items[] }"
          auto& valueReference = mBufferMap.at(paramNode);
          stream << "#define " << param->mName << ' ' << valueReference->mName << 
            "_items" << endl;
        }
        else if (IsPointerOf<StubNode>(paramNode)) {
          /// Parameter is the result of a former function call
          auto& stubReference = shaderStage->mStubMap.at(paramNode);
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
      const auto stubReference = shaderStage->mStubMap.at(node);

      stream << "#define SHADER " << GetParamTypeString(stubMeta->mReturnType) <<
        ' ' << stubReference->mFunctionName << "()" << endl;

      /// Main shader code
      stream << stubMeta->mStrippedSource;

      /// Undefine SHADER macro and samplers
      stream << "#undef SHADER" << endl;
      for (auto param : stubMeta->mParameters)
      {
        stream << "#undef " << param->mName << endl;
      }
    }
  }
}


void ShaderBuilder::GenerateSourceMain(ShaderStage* shaderStage) const
{
  auto& stream = shaderStage->mSourceStream;

  stream << endl;
  stream << "void main() {" << endl;
  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      const shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      auto& stubReference = shaderStage->mStubMap.at(node);

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
