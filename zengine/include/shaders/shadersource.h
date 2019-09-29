#pragma once

#include "stubnode.h"
#include "../dom/node.h"

/// Shader sources and metadata for the entire rendering pipeline. 
/// Includes all programmable stages, ie. vertex and fragment shaders.
struct ShaderSource {

  /// Metadata for a uniform
  struct Uniform {
    Uniform(string name, shared_ptr<Node> node, 
      GlobalUniformUsage globalType, ValueType type);

    const string mName;
    const shared_ptr<Node> mNode;
    const GlobalUniformUsage mGlobalType;
    const ValueType mType;
  };

  /// Metadata for a sampler
  struct Sampler {
    Sampler(string name, shared_ptr<Node> node, 
      GlobalSamplerUsage globalType, bool isMultiSampler, bool isShadow);

    const string mName;
    const shared_ptr<Node> mNode;
    const GlobalSamplerUsage mGlobalType;
    const bool mIsMultiSampler; // type is "sampler2DMS"
    const bool mIsShadow; // type is "sampler2DShadow"
  };

  /// Metadata for a SSBOs and image2Ds
  struct NamedResource {
    NamedResource(string name, shared_ptr<Node> node);

    const string mName;
    const shared_ptr<Node> mNode;
  };

  ShaderSource(
    vector<Uniform> uniforms,
    vector<Sampler> samplers,
    vector<NamedResource> ssbos,
    string vertexSource, string fragmentSource);

  /// All uniforms from all shader stages merged
  const vector<Uniform> mUniforms;

  /// All samplers from all shader stages merged
  const vector<Sampler> mSamplers;

  /// Shader Storage Buffer Objects
  const vector<NamedResource> mSSBOs;

  /// Generated source code for stages stages
  const string mVertexSource;
  const string mFragmentSource;
};
