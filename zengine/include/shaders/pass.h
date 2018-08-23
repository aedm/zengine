#pragma once

#include "shadersource.h"
#include "../dom/node.h"
#include "../render/drawingapi.h"
#include <map>

using namespace std;

enum class PassType {
  SHADOW,
  SOLID,
};

struct PassUniform {
  UniformId handle;
  Node* node;
  ShaderGlobalType globalType;
  ValueType type;
};

/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
/// It manages the entire render pipeline, including setting a shader program,
/// render states, and connecting pipeline resources.
class Pass: public Node {

  /// Associates a uniform to a Node to take its value from
  struct UniformMapper {
    UniformMapper(const ShaderProgram::Uniform* targetUniform, 
                  const ShaderSource::Uniform* source);
    const ShaderProgram::Uniform* mTarget;
    const ShaderSource::Uniform* mSource;
  };

  /// Associates a sampler to a texture Node to take its value from
  struct SamplerMapper {
    SamplerMapper(const ShaderProgram::Sampler* target, 
                  const ShaderSource::Sampler* source);
    const ShaderProgram::Sampler* mTarget;
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

  /// List of vertex attributes used by the program
  const vector<ShaderProgram::Attribute>& GetUsedAttributes();

protected:
  virtual void HandleMessage(Message* message) override;

  /// Creates the shader program
  virtual void Operate() override;

  void BuildShaderSource();

  /// Generated shader source
  shared_ptr<ShaderSource> mShaderSource;

  /// Generated slots
  vector<shared_ptr<Slot>> mUniformAndSamplerSlots;
  
  /// Compiled and linked shader program
  shared_ptr<ShaderProgram> mShaderProgram;

  /// Mappers from nodes to used uniforms
  vector<UniformMapper>	mUsedUniforms;
  vector<SamplerMapper>	mUsedSamplers;

  /// Client-side uniform buffer. 
  /// Uniforms are assembled in this array and then uploaded to OpenGL
  vector<char> mUniformArray;
};

typedef TypedSlot<Pass> PassSlot;
