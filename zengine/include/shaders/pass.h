#pragma once

#include "shadersource2.h"
#include "../dom/node.h"
#include "../render/drawingapi.h"
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

  StubSlot mFragmentStub;
  StubSlot mVertexStub;

  void Set(Globals* globals);

  /// Returns true if pass can be used
  bool isComplete();

  /// List of attributes used by the program
  const vector<ShaderAttributeDesc>& GetUsedAttributes();

  /// Getter for automatic slots
  const Slot* GetFragmentSourceSlot();
  const Slot* GetVertexSourceSlot();

protected:
  /// automatic slots
  ShaderSourceSlot mFragmentSource;
  ShaderSourceSlot mVertexSource;

  virtual void HandleMessage(Slot* slot, NodeMessage message, 
                             const void* payload) override;

  void BuildRenderPipeline();

  ShaderHandle mHandle;
  vector<PassUniform>	mUniforms;
  vector<PassUniform>	mSamplers;
  vector<ShaderAttributeDesc> mAttributes;
};

typedef TypedSlot<NodeType::PASS, Pass> PassSlot;
