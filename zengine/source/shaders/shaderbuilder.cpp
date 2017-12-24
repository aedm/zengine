#include "shaderbuilder.h"
#include <include/shaders/enginestubs.h>
#include <exception>

shared_ptr<ShaderSource> ShaderBuilder::FromStubs(StubNode* vertexStub,
  StubNode* fragmentStub) {
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


ShaderBuilder::InOutVariable::InOutVariable(ValueType type, const string& name, int layout)
  : mName(name)
  , mType(type)
  , mLayout(layout) {}


ShaderBuilder::ShaderStage::ShaderStage(bool isVertexShader)
  : mIsVertexShader(isVertexShader) {}


ShaderBuilder::ShaderBuilder(StubNode* vertexStub, StubNode* fragmentStub)
  : mVertexStage(true)
  , mFragmentStage(false) {
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


ShaderBuilder::~ShaderBuilder() {}

shared_ptr<ShaderSource> ShaderBuilder::MakeShaderSource() {
  return make_shared<ShaderSource>(mUniforms, mSamplers,
    mVertexStage.mSourceStream.str(),
    mFragmentStage.mSourceStream.str());
}

void ShaderBuilder::CollectInputsAndOutputs(Node* node, ShaderStage* shaderStage) {
  StubNode* stub = SafeCast<StubNode*>(node);

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

void ShaderBuilder::TraverseDependencies(Node* root, ShaderStage* shaderStage,
  set<Node*>& visitedNodes) {
  visitedNodes.insert(root);

  if (IsInsanceOf<StubNode*>(root)) {
    /// Assigns empty stub reference
    StubNode* stubNode = SafeCast<StubNode*>(root);
    stubNode->Update();

    ValueType returnType = stubNode->GetStubMetadata()->returnType;
    shaderStage->mStubMap[root] = make_shared<StubReference>(returnType);

    for (auto slotPair : stubNode->mParameterSlotMap) {
      Node* node = slotPair.second->GetDirectNode();
      if (node == nullptr) {
        WARN("Incomplete shader graph.");
        throw exception();
      }
      if (slotPair.first->mType == ValueType::TEXTURE) {
        TextureNode* textureNode = dynamic_cast<TextureNode*>(node);
        ASSERT(textureNode);
        if (textureNode->Get() != nullptr) {
          shaderStage->mDefines.push_back(*slotPair.first->mName + "_CONNECTED");
        }
      }
      if (visitedNodes.find(node) == visitedNodes.end()) {
        TraverseDependencies(node, shaderStage, visitedNodes);
      }
    }
  }
  else {
    auto ref = make_shared<ValueReference>();
    ValueType type = root->GetValueType();
    ASSERT(type != ValueType::NONE);
    ref->mType = type;
    if (type == ValueType::TEXTURE) {
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


void ShaderBuilder::CollectDependencies(Node* root, ShaderStage* shaderStage) {
  StubNode* uberShader = TheEngineStubs->GetStub("uber");
  TraverseDependencies(uberShader, shaderStage, set<Node*>());
  TraverseDependencies(root, shaderStage, set<Node*>());
}


void ShaderBuilder::ShaderStage::GenerateStubNames() {
  int stubIndex = 0;

  for (Node* node : mDependencies) {
    if (IsInsanceOf<StubNode*>(node)) {
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
  for (Node* node : shaderStage->mDependencies) {
    if (!IsInsanceOf<StubNode*>(node)) continue;

    StubNode* stub = SafeCast<StubNode*>(node);
    StubMetadata* stubMeta = stub->GetStubMetadata();
    if (stubMeta == nullptr) {
      ERR("Can't build shader source.");
      throw exception();
    }

    for (auto global : stubMeta->globals) {
      if (mUsedGlobals.find(global->usage) != mUsedGlobals.end()) continue;
      mUsedGlobals.insert(global->usage);
      if (global->type == ValueType::TEXTURE) {
        mSamplers.push_back(ShaderSource::Sampler(global->name, nullptr, global->usage,
          global->isMultiSampler,
          global->isShadow));
      }
      else {
        mUniforms.push_back(ShaderSource::Uniform(global->name, nullptr, global->usage,
          global->type));
      }
    }
  }
}

void ShaderBuilder::AddLocalsToDependencies() {
  for (auto& it : mUniformMap) {
    mUniforms.push_back(ShaderSource::Uniform(
      it.second->mName, it.first, ShaderGlobalType::LOCAL, it.first->GetValueType()));
  }
  for (auto& it : mSamplerMap) {
    mSamplers.push_back(ShaderSource::Sampler(
      it.second->mName, it.first, ShaderGlobalType::LOCAL, false, false));
  }
}

void ShaderBuilder::GenerateSource(ShaderStage* shaderStage) {
  for (Node* node : shaderStage->mDependencies) {
    if (IsInsanceOf<StubNode*>(node)) {
      CollectInputsAndOutputs(node, shaderStage);
    }
  }

  GenerateSourceHeader(shaderStage);
  GenerateSourceFunctions(shaderStage);
  GenerateSourceMain(shaderStage);
}

void ShaderBuilder::GenerateSourceHeader(ShaderStage* shaderStage) {
  shaderStage->mSourceStream << "#version 430 core" << endl;
  shaderStage->mSourceStream << "#define " <<
    (shaderStage->mIsVertexShader ? "VERTEX_SHADER" : "FRAGMENT_SHADER") << endl;

  /// Inputs
  for (auto var : shaderStage->mInputsMap) {
    shaderStage->mSourceStream << "in " << GetTypeString(var.second->mType) << ' ' <<
      var.second->mName << ';' << endl;
    shaderStage->mInputs.push_back(var.second);
  }

  /// Outputs
  for (auto var : shaderStage->mOutputs) {
    if (var->mLayout >= 0) {
      shaderStage->mSourceStream << "layout (location = " << var->mLayout << ") ";
    }
    shaderStage->mSourceStream << "out " << GetTypeString(var->mType) << " " <<
      var->mName << ";" << endl;
  }

  /// Uniform block
  shaderStage->mSourceStream << "layout(shared) uniform Uniforms {" << endl;
  for (auto uniform : mUniforms) {
    shaderStage->mSourceStream << "  " << GetTypeString(uniform.mType) << " " <<
      uniform.mName << ";" << endl;
  }
  shaderStage->mSourceStream << "};" << endl;

  /// Samplers
  /// They are opaque types, thus cannot be part of uniform buffers.
  for (auto sampler : mSamplers) {
    shaderStage->mSourceStream << "uniform " <<
      GetTypeString(ValueType::TEXTURE, sampler.mIsMultiSampler, sampler.mIsShadow) <<
      ' ' << sampler.mName << ';' << endl;
  }

  /// Defines
  for (auto define : shaderStage->mDefines) {
    shaderStage->mSourceStream << "#define " << define << endl;
  }
}

void ShaderBuilder::GenerateSourceFunctions(ShaderStage* shaderStage) {
  for (Node* node : shaderStage->mDependencies) {
    if (IsInsanceOf<StubNode*>(node)) {
      StubNode* stub = static_cast<StubNode*>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();

      /// Define samplers
      shaderStage->mSourceStream << endl;
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        StubParameter* param = stubMeta->parameters[i];
        if (param->mType == ValueType::TEXTURE) {
          Slot* slot = stub->GetSlotByParameter(param);
          Node* paramNode = slot->GetReferencedNode();
          if (paramNode == nullptr) {
            /// Node not connected to param
            ERR("Sampler not connected");
            throw exception();
          }
          auto valueReference = mSamplerMap.at(paramNode);
          shaderStage->mSourceStream << "#define " << *param->mName << ' ' <<
            valueReference->mName << endl;
        }
      }

      /// Define SHADER function signature
      //NodeData* data = mDataMap.at(node);
      auto stubReference = shaderStage->mStubMap.at(node);

      shaderStage->mSourceStream << "#define SHADER " <<
        GetTypeString(stubMeta->returnType) << ' ' << stubReference->mFunctionName << "(";
      bool isFirstParameter = true;
      for (StubParameter* param : stubMeta->parameters) {
        if (param->mType != ValueType::TEXTURE) {
          if (!isFirstParameter) shaderStage->mSourceStream << ", ";
          shaderStage->mSourceStream << GetTypeString(param->mType) << ' ' <<
            *param->mName;
          isFirstParameter = false;
        }
      }
      shaderStage->mSourceStream << ')' << endl;

      /// Main shader code
      shaderStage->mSourceStream << stubMeta->strippedSource;

      /// Undefine SHADER macro and samplers
      shaderStage->mSourceStream << "#undef SHADER" << endl;
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        StubParameter* param = stubMeta->parameters[i];
        if (param->mType == ValueType::TEXTURE) {
          shaderStage->mSourceStream << "#undef " << *param->mName << endl;
        }
      }
    }
  }
}


void ShaderBuilder::GenerateSourceMain(ShaderStage* shaderStage) {
  auto& stream = shaderStage->mSourceStream;

  stream << endl;
  stream << "void main() {" << endl;
  for (Node* node : shaderStage->mDependencies) {
    if (IsInsanceOf<StubNode*>(node)) {
      StubNode* stub = static_cast<StubNode*>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();
      auto stubReference = shaderStage->mStubMap.at(node);

      stream << "  ";
      if (stubReference->mType != ValueType::NONE) {
        stream << GetTypeString(stubReference->mType) << ' ' <<
          stubReference->mVariableName << " = ";
      }
      stream << stubReference->mFunctionName << "(";
      bool isFirstParameter = true;
      for (StubParameter* param : stubMeta->parameters) {
        if (param->mType != ValueType::TEXTURE) {
          if (!isFirstParameter) stream << ", ";
          Slot* slot = stub->GetSlotByParameter(param);
          Node* paramNode = slot->GetReferencedNode();
          if (paramNode == nullptr) {
            /// Node not connected to param
            ERR("Parameter not connected");
            throw exception();
          }
          if (IsInsanceOf<StubNode*>(paramNode)) {
            /// Call parameter is the result of a former function call
            auto paramStubReference = shaderStage->mStubMap.at(paramNode);
            stream << paramStubReference->mVariableName;
          }
          else {
            /// Call parameter is a uniform
            auto valueReference = mUniformMap.at(paramNode);
            stream << valueReference->mName;
          }
          isFirstParameter = false;
        }
      }
      stream << ");" << endl;
    }
  }
  stream << "}" << endl;
}


const string& ShaderBuilder::GetTypeString(ValueType type, bool isMultiSampler, bool isShadow) {
  static const string sfloat("float");
  static const string svec2("vec2");
  static const string svec3("vec3");
  static const string svec4("vec4");
  static const string suint("uint");
  static const string smatrix44("mat4");
  static const string ssampler2d("sampler2D");
  static const string ssampler2dms("sampler2DMS");
  static const string ssampler2dshadow("sampler2DShadow");
  static const string svoid("void");
  static const string serror("UNKNOWN_TYPE");

  if (isMultiSampler) return ssampler2dms;
  if (isShadow) return ssampler2dshadow;

  switch (type) {
  case ValueType::FLOAT:		  return sfloat;
  case ValueType::VEC2:		  return svec2;
  case ValueType::VEC3:		  return svec3;
  case ValueType::VEC4:		  return svec4;
  case ValueType::UINT:		  return suint;
  case ValueType::MATRIX44:	return smatrix44;
  case ValueType::TEXTURE:		return ssampler2d;
  case ValueType::NONE:		  return svoid;
  default:
    ERR("Unhandled type: %d", type);
    return serror;
  }
}

ShaderBuilder::StubReference::StubReference(ValueType type)
  :mType(type) {}
