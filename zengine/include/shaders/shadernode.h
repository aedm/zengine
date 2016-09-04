#pragma once

#include "stubnode.h"
#include "../dom/node.h"

/// Inputs and output of the shader stage
struct ShaderVariable {
  ShaderVariable(NodeType type, const string& name, int layout = -1);

  /// Variable type
  NodeType type;

  /// Variable name
  string name;
  
  /// Output layout number for G-Buffers
  int layout;
};

/// Uniforms of the shader program
struct ShaderUniform {
  ShaderUniform(NodeType type, const string& name, Node* node,
                      ShaderGlobalType globalType, bool isMultiSampler);

  NodeType type;
  string name;
  Node* node;
  ShaderGlobalType globalType;
  bool isMultiSampler; // type is "sampler2DMS"
};


/// All metadata collected from a shader source.
struct ShaderMetadata {
  ShaderMetadata(
    const vector<OWNERSHIP ShaderVariable*>& inputs,
    const vector<OWNERSHIP ShaderVariable*>& outputs,
    const vector<OWNERSHIP ShaderUniform*>& uniforms,
    const vector<OWNERSHIP ShaderUniform*>& samplers,
    const string& source);
  ~ShaderMetadata();

  const vector<ShaderVariable*>	mInputs;
  const vector<ShaderVariable*>	mOutputs;
  const vector<ShaderUniform*>	mUniforms;
  const vector<ShaderUniform*>	mSamplers;

  string mSource;
};

