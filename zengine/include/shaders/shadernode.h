#pragma once

#include "stubnode.h"
#include "../dom/node.h"

struct ShaderSourceVariable {
  ShaderSourceVariable(NodeType type, const string& name);

  NodeType type;
  string name;
  /// TODO: layout, vertexformat usage etc.
};

struct ShaderSourceUniform {
  ShaderSourceUniform(NodeType type, const string& name, Node* node,
                      ShaderGlobalType globalType);

  NodeType type;
  string name;
  Node* node;
  ShaderGlobalType globalType;
};


/// All metadata collected from a shader source.
struct ShaderSourceMetadata {
  ShaderSourceMetadata(
    const vector<OWNERSHIP ShaderSourceVariable*>& inputs,
    const vector<OWNERSHIP ShaderSourceVariable*>& outputs,
    const vector<OWNERSHIP ShaderSourceUniform*>& uniforms,
    const vector<OWNERSHIP ShaderSourceUniform*>& samplers);
  ~ShaderSourceMetadata();

  const vector<ShaderSourceVariable*>	inputs;
  const vector<ShaderSourceVariable*>	outputs;
  const vector<ShaderSourceUniform*>	uniforms;
  const vector<ShaderSourceUniform*>	samplers;
};


class ShaderNode: public Node {
public:
  ShaderNode();
  virtual ~ShaderNode();

  virtual Node* Clone() const override;
  const string& GetSource() const;
  const ShaderSourceMetadata* GetMetadata() const;
  StubSlot mStub;

protected:
  friend class ShaderSourceBuilder;

  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  ShaderSourceMetadata* metadata;
  string mSource;
};

typedef TypedSlot<NodeType::SHADER_SOURCE, ShaderNode> ShaderSourceSlot;

