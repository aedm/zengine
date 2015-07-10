#pragma once

#include "stubnode.h"
#include "../dom/node.h"

/// Inputs and output of the shader stage
struct ShaderVariable {
  ShaderVariable(NodeType type, const string& name);

  NodeType type;
  string name;
  /// TODO: layout, vertexformat usage etc.
};

/// Uniforms of the shader program
struct ShaderUniform {
  ShaderUniform(NodeType type, const string& name, Node* node,
                      ShaderGlobalType globalType);

  NodeType type;
  string name;
  Node* node;
  ShaderGlobalType globalType;
};


/// All metadata collected from a shader source.
struct ShaderMetadata {
  ShaderMetadata(
    const vector<OWNERSHIP ShaderVariable*>& inputs,
    const vector<OWNERSHIP ShaderVariable*>& outputs,
    const vector<OWNERSHIP ShaderUniform*>& uniforms,
    const vector<OWNERSHIP ShaderUniform*>& samplers);
  ~ShaderMetadata();

  const vector<ShaderVariable*>	inputs;
  const vector<ShaderVariable*>	outputs;
  const vector<ShaderUniform*>	uniforms;
  const vector<ShaderUniform*>	samplers;
};


class ShaderNode: public Node {
public:
  ShaderNode();
  virtual ~ShaderNode();

  const string& GetSource() const;
  const ShaderMetadata* GetMetadata() const;
  StubSlot mStub;

protected:
  friend class ShaderBuilder;

  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  ShaderMetadata* metadata;
  string mSource;
};

typedef TypedSlot<NodeType::SHADER_SOURCE, ShaderNode> ShaderSlot;

