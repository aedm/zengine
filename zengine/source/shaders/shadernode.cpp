#include <include/shaders/shadernode.h>
#include <include/shaders/stubnode.h>

ShaderMetadata::ShaderMetadata(
  const vector<OWNERSHIP ShaderVariable*>& inputs,
  const vector<OWNERSHIP ShaderVariable*>& outputs,
  const vector<OWNERSHIP ShaderUniform*>& uniforms,
  const vector<OWNERSHIP ShaderUniform*>& samplers,
  const string& source)
  : mInputs(inputs)
  , mOutputs(outputs)
  , mUniforms(uniforms)
  , mSamplers(samplers)
  , mSource(source)
{}

ShaderMetadata::~ShaderMetadata() {
  for (auto x : mInputs) delete x;
  for (auto x : mOutputs) delete x;
  for (auto x : mUniforms) delete x;
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
