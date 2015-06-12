#pragma once

#include "shadersource2.h"
#include "../dom/node.h"
#include "../shaders/shaders.h"
#include <map>

using namespace std;

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

  virtual Node* Clone() const override;

  Slot mFragmentStub;
  Slot mVertexStub;

  /// Hidden, automatic slots
  Slot mFragmentSource;
  Slot mVertexSource;

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete();

  const vector<ShaderAttributeDesc>& GetUsedAttributes();

protected:
  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  void BuildRenderPipeline();

  ShaderHandle mHandle;
  vector<PassUniform>	mUniforms;
  vector<PassUniform>	mSamplers;
  vector<ShaderAttributeDesc> mAttributes;
};