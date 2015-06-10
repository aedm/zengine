#pragma once

#include "../dom/node.h"
#include "../shaders/shaders.h"
#include <vector>
#include <map>

using namespace std;
class ShaderSource2;

enum class ShaderGlobalType {
#undef ITEM
#define ITEM(name, type, variable) name,
  GLOBALUSAGE_LIST

  LOCAL,	/// For non-global uniforms
};

/// Shader parameter, becomes a slot
/// ":param vec4 MyColor" or ":param sampler2d MyTexture"
struct ShaderStubParameter {
  NodeType type;
  string name;
};

/// Shader variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct ShaderStubVariable {
  NodeType type;
  string name;
};

/// --
struct ShaderStubGlobal {
  NodeType type;
  string name;
  ShaderGlobalType usage;
};


/// All metadata collected from a shader stub source.
struct ShaderStubMetadata {
  ShaderStubMetadata(const string& name, NodeType returnType,
                     const string& strippedSource,
                     const vector<OWNERSHIP ShaderStubParameter*>& parameters,
                     const vector<ShaderStubGlobal*>& globals,
                     const vector<ShaderStubVariable*>& inputs,
                     const vector<ShaderStubVariable*>& outputs);

  ~ShaderStubMetadata();

  const string name;
  const NodeType returnType;

  const string strippedSource;

  const vector<ShaderStubParameter*> parameters;
  const vector<ShaderStubGlobal*> globals;
  const vector<ShaderStubVariable*>	inputs;
  const vector<ShaderStubVariable*>	outputs;
};


class ShaderStub: public Node {
public:
  ShaderStub(const string& source);
  ShaderStub(const ShaderStub& original);
  virtual ~ShaderStub();

  virtual Node* Clone() const;

  void SetStubSource(const string& source);
  const string& GetStubSource() const;
  ShaderStubMetadata* GetStubMetadata() const;

  ShaderSource2* GetShaderSource();
  const map<ShaderStubParameter*, Slot*>& GetParameterSlotMap();

protected:
  /// Handle received messages
  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  /// Metadata
  ShaderStubMetadata* mMetadata;

  string mSource;

  ShaderSource2* mShaderSrc;

  /// Maps stub parameters to stub slots
  map<ShaderStubParameter*, Slot*> mParameterSlotMap;
};


