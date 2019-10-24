#pragma once

#include "stubnode.h"
#include "../dom/node.h"

/// Shader sources and metadata for the entire rendering pipeline. 
/// Includes all programmable stages, ie. vertex and fragment shaders.
struct ShaderSource {

  /// Metadata for a uniform
  struct Uniform {
    Uniform(std::string name, std::shared_ptr<Node> node, 
      GlobalUniformUsage globalType, ValueType type);

    const std::string mName;
    const std::shared_ptr<Node> mNode;
    const GlobalUniformUsage mGlobalType;
    const ValueType mType;
  };

  /// Metadata for a sampler
  struct Sampler {
    Sampler(std::string name, std::shared_ptr<Node> node, 
      GlobalSamplerUsage globalType, bool isMultiSampler, bool isShadow);

    const std::string mName;
    const std::shared_ptr<Node> mNode;
    const GlobalSamplerUsage mGlobalType;
    const bool mIsMultiSampler; // type is "sampler2DMS"
    const bool mIsShadow; // type is "sampler2DShadow"
  };

  /// Metadata for a SSBOs and image2Ds
  struct NamedResource {
    NamedResource(std::string name, std::shared_ptr<Node> node);

    const std::string mName;
    const std::shared_ptr<Node> mNode;
  };

  ShaderSource(
    std::vector<Uniform> uniforms,
    std::vector<Sampler> samplers,
    std::vector<NamedResource> ssbos,
    std::string vertexSource, std::string fragmentSource);

  /// All uniforms from all shader stages merged
  const std::vector<Uniform> mUniforms;

  /// All samplers from all shader stages merged
  const std::vector<Sampler> mSamplers;

  /// Shader Storage Buffer Objects
  const std::vector<NamedResource> mSSBOs;

  /// Generated source code for stages stages
  const std::string mVertexSource;
  const std::string mFragmentSource;
};
