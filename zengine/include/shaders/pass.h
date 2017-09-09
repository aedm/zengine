#pragma once

#include "shadernode.h"
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
class Pass: public Node {
public:
  Pass();
  virtual ~Pass();

  StubSlot mFragmentStub;
  StubSlot mVertexStub;
  StubSlot mUberShader;
  FloatSlot mFaceModeSlot;
  FloatSlot mBlendModeSlot;

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete();

  /// List of vertex attributes used by the program
  const vector<ShaderAttributeDesc>& GetUsedAttributes();

  RenderState mRenderstate;

protected:
  virtual void HandleMessage(Message* message) override;

  /// Creates the shader program
  virtual void Operate() override;

  /// Removes all hidden uniform slots
  void RemoveUniformSlots();

  ShaderHandle mHandle;
  vector<PassUniform>	mUniforms;
  vector<PassUniform>	mSamplers;
  vector<ShaderAttributeDesc> mAttributes;

  ShaderMetadata* mFragmentShaderMetadata = nullptr;
  ShaderMetadata* mVertexShaderMetadata = nullptr;

  vector<Slot*> mUniformAndSamplerSlots;
};

typedef TypedSlot<NodeType::PASS, Pass> PassSlot;
