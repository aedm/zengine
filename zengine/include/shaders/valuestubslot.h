#pragma once

#include "../nodes/valuenodes.h"

template <NodeType T>
class ValueStubSlot: public ValueSlot<T> {
public:
  ValueStubSlot(Node* owner, SharedString name);
  virtual bool DoesAcceptType(NodeType type) override;
};


template <NodeType T>
ValueStubSlot<T>::ValueStubSlot(Node* owner, SharedString name)
  : ValueSlot(owner, name)
  , mType(NodeType::SHADER_STUB)
{}


template <NodeType T>
bool ValueStubSlot<T>::DoesAcceptType(NodeType type) {
  return type == NodeType::SHADER_STUB || type == mType;
}
