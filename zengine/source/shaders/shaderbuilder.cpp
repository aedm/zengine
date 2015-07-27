#include "shaderbuilder.h"
#include <exception>

OWNERSHIP ShaderMetadata* ShaderBuilder::FromStub(StubNode* stub) {
  if (stub == nullptr) {
    ERR("stub is nullptr");
    return nullptr;
  }

  ShaderBuilder builder(stub);
  return new ShaderMetadata(builder.mInputs, builder.mOutputs, builder.mUniforms, 
                            builder.mSamplers, builder.sourceStream.str());
}

ShaderBuilder::ShaderBuilder(StubNode* stub) {
  StubMetadata* stubMeta = stub->GetStubMetadata();
  if (stubMeta == nullptr) {
    ERR("stub has no metadata.");
    return;
  }

  INFO("Building shader source for '%s'...", stubMeta->name.c_str());

  try {
    CollectDependencies(stub);
    GenerateNames();
    for (Node* node : mDependencies) {
      if (node->GetType() == NodeType::SHADER_STUB) {
        CollectStubMetadata(node);
      } else {
        ///NOT_IMPLEMENTED;
        /// TODO: generate uniforms here
      }
    }
    GenerateSlots();
    GenerateSource();
  } catch (...) {
    ERR("Shader source creation failed");
  }
}

ShaderBuilder::~ShaderBuilder() {

}

void ShaderBuilder::CollectStubMetadata(Node* node) {
  StubNode* stub = static_cast<StubNode*>(node);

  StubMetadata* stubMeta = stub->GetStubMetadata();
  if (stubMeta == nullptr) {
    ERR("Can't build shader source.");
    throw exception();
  }

  NodeData* data = mDataMap.at(node);
  data->ReturnType = stubMeta->returnType;

  /// Globals
  for (auto global : stubMeta->globals) {
    mUniforms.push_back(new ShaderUniform(
      global->type, global->name, nullptr, global->usage));
  }

  /// Inputs
  for (auto input : stubMeta->inputs) {
    /// TODO: check whether input types match
    if (mInputsMap.find(input->name) == mInputsMap.end()) {
      mInputsMap[input->name] =
        new ShaderVariable(input->type, input->name);
    }
  }

  /// Outputs
  for (auto output : stubMeta->outputs) {
    mOutputs.push_back(new ShaderVariable(output->type, output->name));
  }
}


void ShaderBuilder::CollectDependencies(Node* root) {
  NodeData* data = new NodeData();
  mDataMap[root] = data;

  if (root->GetType() == NodeType::SHADER_STUB) {
    for (auto slotPair : static_cast<StubNode*>(root)->mParameterSlotMap) {
      Node* node = slotPair.second->GetAbstractNode();
      if (node == nullptr) {
        WARN("Incomplete shader graph.");
        throw exception();
        continue;
      }
      if (mDataMap.find(node) == mDataMap.end()) {
        CollectDependencies(node);
      }
    }
  }

  mDependencies.push_back(root);
}

void ShaderBuilder::GenerateNames() {
  int uniformIndex = 0;
  int stubIndex = 0;
  char tmp[255]; // Fuk c++

  for (Node* node : mDependencies) {
    NodeData* data = mDataMap.at(node);
    if (node->GetType() == NodeType::SHADER_STUB) {
      /// It's a function
      sprintf_s(tmp, "_func_%d", ++stubIndex);
      data->FunctionName = string(tmp);

      sprintf_s(tmp, "_var_%d", stubIndex);
      data->VariableName = string(tmp);
    } else {
      /// It's a uniform
      sprintf_s(tmp, "_uniform_%d", ++uniformIndex);
      data->VariableName = string(tmp);
    }
  }
}

void ShaderBuilder::GenerateSlots() {
  for (Node* node : mDependencies) {
    if (node->GetType() != NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      /// TODO: move this into a separate function
      if (node->GetType() == NodeType::TEXTURE) {
        mSamplers.push_back(new ShaderUniform(
          node->GetType(), data->VariableName, node, ShaderGlobalType::LOCAL));
      } else {
        mUniforms.push_back(new ShaderUniform(
          node->GetType(), data->VariableName, node, ShaderGlobalType::LOCAL));
      }
    }
  }
}

void ShaderBuilder::GenerateSource() {
  sourceStream << "#version 150" << endl;
  GenerateSourceHeader(sourceStream);
  GenerateSourceFunctions(sourceStream);
  GenerateSourceMain(sourceStream);
}

void ShaderBuilder::GenerateSourceHeader(stringstream& stream) {
  /// Inputs
  for (auto var : mInputsMap) {
    stream << "in " << GetTypeString(var.second->type) << ' ' <<
      var.second->name << ';' << endl;
    mInputs.push_back(var.second);
  }

  /// Outputs
  for (auto var : mOutputs) {
    stream << "out " << GetTypeString(var->type) << ' ' <<
      var->name << ';' << endl;
  }

  /// Uniforms
  for (auto uniform : mUniforms) {
    stream << "uniform " << GetTypeString(uniform->type) << ' ' <<
      uniform->name << ';' << endl;
  }

  /// Samplers
  for (auto uniform : mSamplers) {
    stream << "uniform " << GetTypeString(uniform->type) << ' ' <<
      uniform->name << ';' << endl;
  }
}

void ShaderBuilder::GenerateSourceFunctions(stringstream& stream) {
  for (Node* node : mDependencies) {
    if (node->GetType() == NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      StubNode* stub = static_cast<StubNode*>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();

      /// Define samplers
      stream << endl;
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        StubParameter* param = stubMeta->parameters[i];
        if (param->type == NodeType::TEXTURE) {
          Slot* slot = stub->GetSlotByParameter(param);
          Node* paramNode = slot->GetAbstractNode();
          if (paramNode == nullptr) {
            /// Node not connected to param
            ERR("Sampler not connected");
            throw exception();
          }
          NodeData* samplerData = mDataMap.at(paramNode);
          stream << "#define " << param->name << ' ' << 
            samplerData->VariableName << endl;
        }
      }

      /// Define SHADER function signature
      stream << "#define SHADER " << GetTypeString(stubMeta->returnType) <<
        ' ' << data->FunctionName << "(";
      bool isFirstParameter = true;
      for (StubParameter* param : stubMeta->parameters) {
        if (param->type != NodeType::TEXTURE) {
          if (!isFirstParameter) stream << ", ";
          stream << GetTypeString(param->type) << ' ' << param->name;
          isFirstParameter = false;
        }
      }
      stream << ')' << endl;

      /// Main shader code
      stream << stubMeta->strippedSource;

      /// Undefine SHADER macro and samplers
      stream << "#undef SHADER" << endl;
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        StubParameter* param = stubMeta->parameters[i];
        if (param->type == NodeType::TEXTURE) {
          stream << "#undef " << param->name << endl;
        }
      }
    }
  }
}


void ShaderBuilder::GenerateSourceMain(stringstream& stream) {
  stream << endl;
  stream << "void main() {" << endl;
  for (Node* node : mDependencies) {
    if (node->GetType() == NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      StubNode* stub = static_cast<StubNode*>(node);
      StubMetadata* stubMeta = stub->GetStubMetadata();

      stream << "  ";
      if (data->ReturnType != NodeType::NONE) {
        stream << GetTypeString(data->ReturnType) << ' ' <<
          data->VariableName << " = ";
      }
      stream << data->FunctionName << "(";
      bool isFirstParameter = true;
      for (StubParameter* param : stubMeta->parameters) {
        if (param->type != NodeType::TEXTURE) {
          Slot* slot = stub->GetSlotByParameter(param);
          Node* paramNode = slot->GetAbstractNode();
          if (paramNode == nullptr) {
            /// Node not connected to param
            ERR("Parameter not connected");
            throw exception();
          }
          NodeData* paramData = mDataMap.at(paramNode);
          if (!isFirstParameter) stream << ", ";
          stream << paramData->VariableName;
          isFirstParameter = false;
        }
      }
      stream << ");" << endl;
    }
  }
  stream << "}" << endl;
}


const string& ShaderBuilder::GetTypeString(NodeType type) {
  static const string sfloat("float");
  static const string svec2("vec2");
  static const string svec3("vec3");
  static const string svec4("vec4");
  static const string suint("uint");
  static const string smatrix44("mat4");
  static const string ssampler2d("sampler2D");
  static const string svoid("void");
  static const string serror("UNKNOWN_TYPE");

  switch (type) {
    case NodeType::FLOAT:		  return sfloat;
    case NodeType::VEC2:		  return svec2;
    case NodeType::VEC3:		  return svec3;
    case NodeType::VEC4:		  return svec4;
    case NodeType::UINT:		  return suint;
    case NodeType::MATRIX44:	return smatrix44;
    case NodeType::TEXTURE:		return ssampler2d;
    case NodeType::NONE:		  return svoid;
    default:
      ERR("Unhandled type: %d", type);
      return serror;
  }
}
