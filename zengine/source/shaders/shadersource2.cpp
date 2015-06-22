#include <include/shaders/shadersource2.h>
#include "shadersourcebuilder.h"
#include <include/shaders/shaderstub.h>

ShaderSource2::ShaderSource2()
  : Node(NodeType::SHADER_SOURCE, "ShaderSource")
  , mStub(NodeType::SHADER_STUB, this, make_shared<string>("Stub"))
  , metadata(nullptr) {}


ShaderSource2::~ShaderSource2() {
  SafeDelete(metadata);
  /// There are dynamically created slots that won't be automatically 
  /// disconencted.
  for (Slot* slot : mSlots) {
    slot->DisconnectAll(false);
  }
}


Node* ShaderSource2::Clone() const {
  /// Shader sources get all their data from the slot, so nothing to do here.
  return new ShaderSource2();
}


void ShaderSource2::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
      if (slot == &mStub) {
        ShaderSourceBuilder::FromStub(static_cast<ShaderStub*>(mStub.GetNode()), this);
        SendMsg(NodeMessage::VALUE_CHANGED, nullptr);
      }
      break;
    case NodeMessage::VALUE_CHANGED:
      SendMsg(NodeMessage::NEEDS_REDRAW, nullptr);
      break;
    default:
      break;
  }
}

const string& ShaderSource2::GetSource() const {
  return mSource;
}

const ShaderSourceMetadata* ShaderSource2::GetMetadata() const {
  return metadata;
}


ShaderSourceMetadata::ShaderSourceMetadata(
  const vector<OWNERSHIP ShaderSourceVariable*>& _Inputs,
  const vector<OWNERSHIP ShaderSourceVariable*>& _Outputs,
  const vector<OWNERSHIP ShaderSourceUniform*>& _Uniforms,
  const vector<OWNERSHIP ShaderSourceUniform*>& _samplers)
  : inputs(_Inputs)
  , outputs(_Outputs)
  , uniforms(_Uniforms)
  , samplers(_samplers)
{}

ShaderSourceMetadata::~ShaderSourceMetadata() {
  for (auto x : inputs) delete x;
  for (auto x : outputs) delete x;
  for (auto x : uniforms) delete x;
}


ShaderSourceVariable::ShaderSourceVariable(NodeType _type, const string& _name)
  : name(_name)
  , type(_type) {}

ShaderSourceUniform::ShaderSourceUniform(NodeType _type, const string& _name, Node* _node,
                                         ShaderGlobalType _globalType)
  : type(_type)
  , name(_name)
  , node(_node)
  , globalType(_globalType) {}
