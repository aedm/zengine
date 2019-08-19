#pragma once

#include "shadersource.h"
#include "../dom/node.h"
#include "../render/drawingapi.h"
#include <map>

using namespace std;

enum class PassType {
  SHADOW,
  SOLID,
  ZPOST,
};

/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
/// It manages the entire render pipeline, including setting a shader program,
/// render states, and connecting pipeline resources.
class Pass: public Node {

  /// Associates a uniform to a Node to take its value from
  struct UniformMapper {
    UniformMapper(const ShaderCompiledStage::Uniform* targetUniform,
                  const ShaderSource::Uniform* source);
    const ShaderCompiledStage::Uniform* mTarget;
    const ShaderSource::Uniform* mSource;
  };

  /// Associates a sampler to a texture Node to take its value from
  struct SamplerMapper {
    SamplerMapper(const ShaderCompiledStage::Sampler* target,
                  const ShaderSource::Sampler* source);
    const ShaderCompiledStage::Sampler* mTarget;
    const ShaderSource::Sampler* mSource;
  };

public:
  Pass();

  /// Shader stages and sources
  StubSlot mVertexStub;
  StubSlot mFragmentStub;
  StubSlot mUberShader;

  /// Pipeline state
  FloatSlot mFaceModeSlot;
  FloatSlot mBlendModeSlot;
  RenderState mRenderstate;

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete();

protected:
  virtual void HandleMessage(Message* message) override;

  /// Creates the shader program
  virtual void Operate() override;

  void BuildShaderSource();

  /// Collect all uniform data from nodes and globals for a UBO
  void FillUniformArray(vector<UniformMapper>& uniforms, Globals* globals, 
    vector<char>& uniformArray);
  void CollectUniformsAndSamplersFromShaderStage(
    const shared_ptr<ShaderCompiledStage>& shaderStage, 
    vector<UniformMapper>& uniforms, vector<SamplerMapper>& samplers);

  /// Generated shader source
  shared_ptr<ShaderSource> mVertexShaderSource;
  shared_ptr<ShaderSource> mFragmentShaderSource;

  /// Generated slots
  vector<shared_ptr<Slot>> mUniformAndSamplerSlots;
  
  /// Compiled and linked shader program
  shared_ptr<ShaderProgram> mShaderProgram;

  /// Mappers from nodes to used uniforms
  vector<UniformMapper>	mUsedVertexUniforms;
  vector<UniformMapper>	mUsedFragmentUniforms;
  vector<SamplerMapper>	mUsedSamplers;

  /// Client-side uniform buffer. 
  /// Uniforms are assembled in this array and then uploaded to OpenGL
  vector<char> mVertexUniformArray;
  vector<char> mFragmentUniformArray;
  
};

typedef TypedSlot<Pass> PassSlot;
