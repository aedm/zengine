#include <include/shaders/shadernode.h>
#include "shaderbuilder.h"
#include <include/shaders/stubnode.h>

static SharedString StubSlotName = make_shared<string>("Stub");

ShaderNode::ShaderNode()
  : Node(NodeType::SHADER_SOURCE)
  , mStub(this, StubSlotName)
  , metadata(nullptr) {}


ShaderNode::~ShaderNode() {
  SafeDelete(metadata);
  /// There are dynamically created slots that won't be automatically 
  /// disconencted.
  for (Slot* slot : GetPublicSlots()) {
    delete slot;
  }
}


Node* ShaderNode::Clone() const {
  /// Shader sources get all their data from the slot, so nothing to do here.
  return new ShaderNode();
}


void ShaderNode::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
    case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
      if (slot == &mStub) {
        ShaderBuilder::FromStub(mStub.GetNode(), this);
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

const string& ShaderNode::GetSource() const {
  return mSource;
}

const ShaderMetadata* ShaderNode::GetMetadata() const {
  return metadata;
}


ShaderMetadata::ShaderMetadata(
  const vector<OWNERSHIP ShaderVariable*>& _Inputs,
  const vector<OWNERSHIP ShaderVariable*>& _Outputs,
  const vector<OWNERSHIP ShaderUniform*>& _Uniforms,
  const vector<OWNERSHIP ShaderUniform*>& _samplers)
  : inputs(_Inputs)
  , outputs(_Outputs)
  , uniforms(_Uniforms)
  , samplers(_samplers)
{}

ShaderMetadata::~ShaderMetadata() {
  for (auto x : inputs) delete x;
  for (auto x : outputs) delete x;
  for (auto x : uniforms) delete x;
}


ShaderVariable::ShaderVariable(NodeType _type, const string& _name)
  : name(_name)
  , type(_type) {}

ShaderUniform::ShaderUniform(NodeType _type, const string& _name, Node* _node,
                                         ShaderGlobalType _globalType)
  : type(_type)
  , name(_name)
  , node(_node)
  , globalType(_globalType) {}
