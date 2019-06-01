#include <include/shaders/shadersource.h>
#include <include/shaders/stubnode.h>

ShaderSource::ShaderSource(
  const vector<Uniform>& uniforms,
  const vector<Sampler>& samplers,
  const string& vertexSource, const string& fragmentSource)
  : mUniforms(uniforms)
  , mSamplers(samplers)
  , mVertexSource(vertexSource)
  , mFragmentSource(fragmentSource)
{}

ShaderSource::Uniform::Uniform(const string& name, const shared_ptr<Node>& node,
  GlobalUniformUsage globalType, ShaderValueType type)
  : mName(name)
  , mNode(node)
  , mGlobalType(globalType)
  , mType(type)
{}

ShaderSource::Sampler::Sampler(const string& name, const shared_ptr<Node>& node,
  GlobalSamplerUsage globalType, bool isMultiSampler, bool isShadow)
  : mName(name)
  , mNode(node)
  , mGlobalType(globalType)
  , mIsMultiSampler(isMultiSampler)
  , mIsShadow(isShadow)
{}
