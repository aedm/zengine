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
  NodeType type;
};

/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
/// It manages the entire render pipeline, including setting a shader program,
/// render states, and connecting pipeline resources.
class Pass: public Node {

  /// Associates a uniform to a Node to take its value from
  class UniformMapper {
    ShaderProgram::Uniform* mTargetUniform;
    Node* mSourceNode;
  };

  /// Associates a sampler to a texture Node to take its value from
  class SamplerMapper {
    ShaderProgram::Sampler* mTargetSampler;
    Node* mSourceNode;
  };

public:
  Pass();
  virtual ~Pass();

  /// Shader stages and sources
  StubSlot mFragmentStub;
  StubSlot mVertexStub;
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

  /// Removes all hidden uniform slots
  void RemoveUniformSlots();

  vector<UniformMapper>	mUniforms;
  vector<SamplerMapper>	mSamplers;
  vector<ShaderProgram::Attribute> mAttributes;

  //ShaderSource* mFragmentShaderMetadata = nullptr;
  //ShaderSource* mVertexShaderMetadata = nullptr;
  ShaderSource* mShaderSource = nullptr;

  vector<Slot*> mUniformAndSamplerSlots;
  
  ShaderHandle mHandle;
};

typedef TypedSlot<NodeType::PASS, Pass> PassSlot;
