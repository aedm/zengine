#include "shadersourcebuilder.h"
#include <exception>

void ShaderSourceBuilder::FromStub(ShaderStub* stub, ShaderSource2* source) {
  ShaderSourceBuilder builder(stub, source);
}

ShaderSourceBuilder::ShaderSourceBuilder(ShaderStub* stub, ShaderSource2* source)
  : mSource(source) {
  INFO("Building shader source...");
  SafeDelete(mSource->metadata);
  for (Slot* slot : mSource->mSlots) {
    if (slot != &mSource->mStub) delete slot;
  }
  mSource->mSlots.clear();
  mSource->mSlots.push_back(&mSource->mStub);

  if (stub == nullptr) return;

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
    source->metadata = new ShaderSourceMetadata(mInputs, mOutputs, mUniforms);
  } catch (...) {
  }
}

ShaderSourceBuilder::~ShaderSourceBuilder() {

}

void ShaderSourceBuilder::CollectStubMetadata(Node* node) {
  ShaderStub* stub = static_cast<ShaderStub*>(node);

  ShaderStubMetadata* stubMeta = stub->GetStubMetadata();
  NodeData* data = mDataMap.at(node);
  data->ReturnType = stubMeta->returnType;

  const map<ShaderStubParameter*, Slot*>& paramSlotMap = stub->GetParameterSlotMap();

  /// Globals
  for (auto global : stubMeta->globals) {
    mUniforms.push_back(new ShaderSourceUniform(
      global->type, global->name, nullptr, global->usage));
  }

  /// Inputs
  for (auto input : stubMeta->inputs) {
    /// TODO: check whether input types match
    if (mInputsMap.find(input->name) == mInputsMap.end()) {
      mInputsMap[input->name] =
        new ShaderSourceVariable(input->type, input->name);
    }
  }

  /// Outputs
  for (auto output : stubMeta->outputs) {
    mOutputs.push_back(new ShaderSourceVariable(output->type, output->name));
  }
}


void ShaderSourceBuilder::CollectDependencies(Node* root) {
  NodeData* data = new NodeData();
  mDataMap[root] = data;

  if (root->GetType() == NodeType::SHADER_STUB) {
    for (Slot* slot : root->mSlots) {
      Node* node = slot->GetNode();
      if (node == nullptr) {
        WARN("Incomplete shader graph.");
        throw exception();
      }
      if (mDataMap.find(node) == mDataMap.end()) {
        CollectDependencies(node);
      }
    }
  }

  mDependencies.push_back(root);
}

void ShaderSourceBuilder::GenerateNames() {
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

void ShaderSourceBuilder::GenerateSourceMetadata() {
  vector<ShaderSourceVariable*> inputs;
  for (auto input : mInputsMap) {
    inputs.push_back(input.second);
  }
  mSource->metadata = new ShaderSourceMetadata(inputs, mOutputs, mUniforms);
}

void ShaderSourceBuilder::GenerateSlots() {
  for (Node* node : mDependencies) {
    if (node->GetType() != NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      Slot* slot = new Slot(node->GetType(), mSource, nullptr, false, true);
      slot->Connect(node);
      mUniforms.push_back(new ShaderSourceUniform(
        node->GetType(), data->VariableName, node, ShaderGlobalType::LOCAL));
    }
  }

  mSource->ReceiveMessage(nullptr, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);
}

void ShaderSourceBuilder::GenerateSource() {
  mSource->mSource.clear();
  stringstream stream;
  stream << "#version 150" << endl;

  GenerateSourceHeader(stream);
  GenerateSourceFunctions(stream);
  GenerateSourceMain(stream);

  mSource->mSource = stream.str();
}

void ShaderSourceBuilder::GenerateSourceHeader(stringstream& stream) {
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
}

void ShaderSourceBuilder::GenerateSourceFunctions(stringstream& stream) {
  for (Node* node : mDependencies) {
    if (node->GetType() == NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      ShaderStub* stub = static_cast<ShaderStub*>(node);
      ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

      stream << endl;
      stream << "#define SHADER " << GetTypeString(stubMeta->returnType) <<
        ' ' << data->FunctionName << "(";
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        ShaderStubParameter* param = stubMeta->parameters[i];
        stream << GetTypeString(param->type) << ' ' << param->name;
        if (i < stubMeta->parameters.size() - 1) stream << ", ";
      }
      stream << ')' << endl;
      stream << stubMeta->strippedSource;
      stream << "#undef SHADER" << endl;
    }
  }
}


void ShaderSourceBuilder::GenerateSourceMain(stringstream& stream) {
  stream << endl;
  stream << "void main() {" << endl;
  for (Node* node : mDependencies) {
    if (node->GetType() == NodeType::SHADER_STUB) {
      NodeData* data = mDataMap.at(node);
      ShaderStub* stub = static_cast<ShaderStub*>(node);
      ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

      stream << "  ";
      if (data->ReturnType != NodeType::NONE) {
        stream << GetTypeString(data->ReturnType) << ' ' <<
          data->VariableName << " = ";
      }
      stream << data->FunctionName << "(";
      for (UINT i = 0; i < stubMeta->parameters.size(); i++) {
        ShaderStubParameter* param = stubMeta->parameters[i];
        Slot* slot = stub->GetParameterSlotMap().at(param);
        Node* paramNode = slot->GetNode();
        if (paramNode == nullptr) {
          /// Node not connected to param
          ERR("Parameter not connected");
          throw exception();
        }
        NodeData* paramData = mDataMap.at(paramNode);
        stream << paramData->VariableName;
        if (i < stubMeta->parameters.size() - 1) stream << ", ";
      }
      stream << ");" << endl;
    }
  }
  stream << "}" << endl;
}


const string& ShaderSourceBuilder::GetTypeString(NodeType type) {
  static const string sfloat("float");
  static const string svec2("vec2");
  static const string svec3("vec3");
  static const string svec4("vec4");
  static const string suint("uint");
  static const string smatrix44("mat4");
  static const string ssampler2d("sampler2d");
  static const string svoid("void");
  static const string serror("UNKNOWN_TYPE");

  switch (type) {
    case NodeType::FLOAT:		return sfloat;
    case NodeType::VEC2:		return svec2;
    case NodeType::VEC3:		return svec3;
    case NodeType::VEC4:		return svec4;
    case NodeType::UINT:		return suint;
    case NodeType::MATRIX44:	return smatrix44;
    case NodeType::TEXTURE:		return ssampler2d;
    case NodeType::NONE:		return svoid;
    default:
      ERR("Unhandled type: %d", type);
      return serror;
  }
}
