#include <include/shaders/shadersource.h>
#include <include/shaders/stubnode.h>
#include <utility>

ShaderSource::ShaderSource(
  std::vector<Uniform> uniforms,
  std::vector<Sampler> samplers,
  std::vector<NamedResource> ssbos,
  std::string vertexSource, std::string fragmentSource)
  : mUniforms(std::move(uniforms))
  , mSamplers(std::move(samplers))
  , mSSBOs(std::move(ssbos))
  , mVertexSource(std::move(vertexSource))
  , mFragmentSource(std::move(fragmentSource))
{}

ShaderSource::Uniform::Uniform(std::string name, std::shared_ptr<Node> node,
  GlobalUniformUsage globalType, ValueType type)
  : mName(std::move(name))
  , mNode(std::move(node))
  , mGlobalType(globalType)
  , mType(type)
{}

ShaderSource::Sampler::Sampler(std::string name, std::shared_ptr<Node> node,
  GlobalSamplerUsage globalType, bool isMultiSampler, bool isShadow)
  : mName(std::move(name))
  , mNode(std::move(node))
  , mGlobalType(globalType)
  , mIsMultiSampler(isMultiSampler)
  , mIsShadow(isShadow)
{}

ShaderSource::NamedResource::NamedResource(std::string name,
                                           std::shared_ptr<Node> node)
  : mName(std::move(name))
  , mNode(std::move(node)) {}
