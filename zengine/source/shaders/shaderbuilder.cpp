#include "shaderbuilder.h"
#include <include/shaders/enginestubs.h>
#include <include/nodes/texturenode.h>
#include <exception>

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


ShaderBuilder::InOutVariable::InOutVariable(ValueType type, const string& name, 
  int layout)
  : mName(name)
  , mType(type)
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
  INFO("Analyzing stubs, vs: '%s', fs: '%s' ...", vertexStubMeta->name.c_str(),
    fragmentStubMeta->name.c_str());

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

    INFO("Building vertex shader source for '%s'...", vertexStubMeta->name.c_str());
    GenerateSource(&mVertexStage);

    INFO("Building fragment shader source for '%s'...", fragmentStubMeta->name.c_str());
    GenerateSource(&mFragmentStage);
  }
  catch (...) {
    ERR("Shader source creation failed");
  }
}

shared_ptr<ShaderSource> ShaderBuilder::MakeShaderSource() {
  return make_shared<ShaderSource>(mUniforms, mSamplers,
    mVertexStage.mSourceStream.str(),
    mFragmentStage.mSourceStream.str());
}

void ShaderBuilder::CollectInputsAndOutputs(
  const shared_ptr<Node>& node, ShaderStage* shaderStage)
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
    if (shaderStage->mInputsMap.find(input->name) == shaderStage->mInputsMap.end()) {
      shaderStage->mInputsMap[input->name] =
        make_shared<InOutVariable>(input->type, input->name);
    }
  }

  /// Outputs
  for (auto output : stubMeta->outputs) {
    int layout = -1;
    if (output->name == "GBufferTargetA") layout = 0;
    else if (output->name == "GBufferTargetB") layout = 1;
    else if (output->name == "GBufferTargetC") layout = 2;
    else if (output->name == "GBufferTargetD") layout = 3;
    shaderStage->mOutputs.push_back(
      make_shared<InOutVariable>(output->type, output->name, layout));
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
  ShaderStage* shaderStage, set<shared_ptr<Node>>& visitedNodes)
{
  visitedNodes.insert(root);

  if (IsPointerOf<StubNode>(root)) {
    /// Assigns empty stub reference
    auto stubNode = PointerCast<StubNode>(root);
    stubNode->Update();

    StubParameter::Type returnType = stubNode->GetStubMetadata()->returnType;
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

  shaderStage->mDependencies.push_back(root);
}


void ShaderBuilder::CollectDependencies(const shared_ptr<Node>& root,
  ShaderStage* shaderStage)
{
  shared_ptr<StubNode> uberShader = TheEngineStubs->GetStub("uber");
  TraverseDependencies(uberShader, shaderStage, set<shared_ptr<Node>>());
  TraverseDependencies(root, shaderStage, set<shared_ptr<Node>>());
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
}


void ShaderBuilder::AddGlobalsToDependencies(ShaderStage* shaderStage) {
  for (const auto& node : shaderStage->mDependencies) {
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

void ShaderBuilder::GenerateSourceHeader(ShaderStage* shaderStage) {
  stringstream& stream = shaderStage->mSourceStream;

  stream << "#version 430 core" << endl;
  stream << "#define " <<
    (shaderStage->mIsVertexShader ? "VERTEX_SHADER" : "FRAGMENT_SHADER") << endl;

  /// Inputs
  for (const auto& var : shaderStage->mInputsMap) {
    stream << "in " << GetValueTypeString(var.second->mType) << ' ' << 
      var.second->mName << ';' << endl;
    shaderStage->mInputs.push_back(var.second);
  }

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
  for (auto sampler : mSamplers) {
    stream << "uniform " <<
      GetParamTypeString(StubParameter::Type::SAMPLER2D, sampler.mIsMultiSampler, 
        sampler.mIsShadow) << ' ' << sampler.mName << ';' << endl;
  }

  /// Stub inputs as variables
  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      auto& stubReference = shaderStage->mStubMap.at(node);

      if (stubReference->mType != StubParameter::Type::TVOID) {
        stream << GetParamTypeString(stubReference->mType) << ' ' <<
          stubReference->mVariableName << ";" << endl;
      }
    }
  }

  /// Defines
  for (auto define : shaderStage->mDefines) {
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
      auto stubReference = shaderStage->mStubMap.at(node);

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


void ShaderBuilder::GenerateSourceMain(ShaderStage* shaderStage) {
  auto& stream = shaderStage->mSourceStream;

  stream << endl;
  stream << "void main() {" << endl;
  for (const auto& node : shaderStage->mDependencies) {
    if (IsPointerOf<StubNode>(node)) {
      shared_ptr<StubNode> stub = PointerCast<StubNode>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
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
